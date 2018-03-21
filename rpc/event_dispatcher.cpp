#include "rpc/event_dispatcher.h"
#include "rpc/selector.h"
#include "rpc/channel.h"
#include "assert.h"
#include "log/logging.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include "rpc/timer/timer_queue_rbtree.h"
#include "rpc/timer/timer_queue_ll.h"
#include "rpc/epoll_selector.h"

static int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG(FATAL) << "Failed in eventfd";
        abort();
    }
    LOG(TRACE) << "create eventfd = " << evtfd;
    return evtfd;
}

class WakeupChannel : public Channel {
    public:
        WakeupChannel(EventDispatcher *evd,int fd) : Channel(evd,fd) {
        }

        virtual void OnRead() {
            uint64_t one = 1;
            ssize_t n = ::read(_fd, &one, sizeof one);
            LOG(TRACE) << " >>> eventfd READ! <<< ";
            if (n != sizeof one)
            {
                LOG(ERROR) << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
            }
            exit(0);
        }
};

EventDispatcher::EventDispatcher():
    //_stop(false),_selector(MakeDefaultSelector(this)),_timer_queue(new TimerQueue_linked_list(this)) {
    _stop(false),_selector(new EpollSelector(this)),_timer_queue(new TimerQueueRbtree(this)) {
    _wakeup_fds[0] = createEventfd();
    // fd[1] is no use
    _wakeup_fds[1] = 0;
    _wakeup_channel.reset(new WakeupChannel(this,_wakeup_fds[0]));

    //_wakeup_channel->SetEvents(EV_READ);
    //_timer_queue->SetEvents(EV_READ);
}

EventDispatcher::~EventDispatcher() {
    Stop();
}

void EventDispatcher::Run() {
    _wakeup_channel->SetEvents(EV_READ);
    _timer_queue->Start();
    vector<Event> events;
    // loop
    for(;;) {
        events.clear();
        _selector->Select(100,events);
        for (auto it = events.begin(); it != events.end(); ++it) {
            Channel *channel = findChannel(it->fd);
            if (nullptr != channel) {
                LOG(TRACE) << "get a channel = " << channel << ",fd = " << it->fd;
                channel->HandleEvent(*it);
            } else {
                LOG(WARNING) << "Found fd which don't have Channel.fd = " << it->fd;
                exit(-1); 
            }
        } 
        runPendingFunctor();
        // FIXME
        if (_stop) {
            break;
        }
    }
}

void EventDispatcher::Stop() {
    _stop = true;
    Wakeup();
}

// Never call in another thread!
int EventDispatcher::RegisterChannel(Channel *channel) {
    int fd = channel->Getfd();
    _selector->Add(fd,channel->GetEvents());
    _ref.insert(std::make_pair(fd,channel));
    return 0;
}

// Never call in another thread!
int EventDispatcher::UpdateChannel(Channel *channel) {
    _selector->Update(channel->Getfd(),channel->GetEvents()); 
    return 0;
}

// Never call in another thread!
int EventDispatcher::RemoveChannel(Channel *channel) {
    auto it = _ref.find(channel->Getfd());
    assert(it->second == channel);

    int fd = channel->Getfd();
    LOG(TRACE) << "Rmove channel. fd = " << fd;

    _selector->Remove(fd);
    _ref.erase(fd);
    return 0;
}

Channel *EventDispatcher::findChannel(int fd) {
    auto it = _ref.find(fd);
    if (it == _ref.end()) {
        return nullptr;
    }
    return it->second;
}

void EventDispatcher::Wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(_wakeup_fds[0], &one, sizeof one);
    if (n != sizeof one)
    {
       LOG(FATAL) << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

// TODO call AddPendingFunctor in Functor ?
void EventDispatcher::AddPendingFunctor(Functor &&cb) {
    {
        std::lock_guard<std::mutex> guard(_mutex);
        _pending_functors.push_back(std::ref(cb));
    }
    Wakeup();
}

void EventDispatcher::runPendingFunctor() {
    std::vector<Functor> functors;
    {
        std::lock_guard<std::mutex> guard(_mutex);
        functors.swap(_pending_functors);
    }
    for (size_t i = 0; i < functors.size(); ++i)
    {
        LOG(TRACE) << "Running functor!!! idx = " << i;
        functors[i]();
    }
}

// Never call in another thread!
int EventDispatcher::AddTimer(int64_t time_ms,int interval,Functor cb) {
    return _timer_queue->AddTimer(time_ms * 1000000UL,interval,cb);
}

// Never call in another thread!
void EventDispatcher::CancelTimer(int timer_id) {
    _timer_queue->CancelTimer(timer_id);
}


