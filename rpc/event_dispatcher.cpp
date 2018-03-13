#include "rpc/event_dispatcher.h"
#include "rpc/selector.h"
#include "rpc/channel.h"
#include "assert.h"
#include "log/logging.h"
#include <sys/eventfd.h>
#include <unistd.h>

static int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG(FATAL) << "Failed in eventfd";
        abort();
    }
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

EventDispatcher::EventDispatcher():_stop(false),_selector(MakeDefaultSelector(this)) {
    _wakeup_fds[0] = createEventfd();
    // fd[1] is no use
    _wakeup_fds[1] = 0;
    _wakeup_channel.reset(new WakeupChannel(this,_wakeup_fds[0]));
}

void EventDispatcher::Run() {
    vector<Event> events;
    // loop
    for(;;) {
        events.clear();
        _selector->Select(100,events);
//        LOG(TRACE) << "Select events.size = " << events.size();
        for (auto it = events.begin(); it != events.end(); ++it) {
            Channel *channel = FindChannel(it->fd);
            if (nullptr != channel) {
                LOG(TRACE) << "get a channel = " << channel << ",fd = " << it->fd;
                channel->HandleEvent(*it);
            } else {
                LOG(WARNING) << "Found fd which don't have Channel.fd = " << it->fd;
                exit(-1); 
            }
        } 
        runPendingFunctor();
        if (_stop) {
            break;
        }
    }
}

void EventDispatcher::Stop() {
    _stop = true;
}

int EventDispatcher::RegisterChannel(Channel *channel) {
    int fd = channel->Getfd();
    _selector->Add(fd,channel->GetEvents());
    _fd_channel_map[fd] = channel;
    return 0;
}

int EventDispatcher::UpdateChannel(Channel *channel) {
    _selector->Update(channel->Getfd(),channel->GetEvents()); 
    return 0;
}

int EventDispatcher::RemoveChannel(Channel *channel) {
    auto it = _fd_channel_map.find(channel->Getfd());
    assert(it->second == channel);

    int fd = channel->Getfd();
    LOG(TRACE) << "Rmove channel. fd = " << fd;

    _selector->Remove(fd);
    _fd_channel_map.erase(fd);
    return 0;
}

Channel *EventDispatcher::FindChannel(int fd) {
    auto it = _fd_channel_map.find(fd);
    if (it == _fd_channel_map.end()) {
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
