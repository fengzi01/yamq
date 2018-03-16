#include "rpc/epoll_selector.h"
#include <sys/epoll.h>
#include "log/logging.h"

// typedef union epoll_data {
//    void *ptr;
//    int fd;
//    __uint32_t u32;
//    __uint64_t u64;
// } epoll_data_t;
//
// struct epoll_event {
//    __uint32_t events; /* Epoll events */
//    epoll_data_t data; /* User data variable */
// };
//
// EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
// EPOLLOUT：表示对应的文件描述符可以写；
// EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
// EPOLLERR：表示对应的文件描述符发生错误；
// EPOLLHUP：表示对应的文件描述符被挂断；
// EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
// EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里

static int _trans_epollev_to_ev(uint32_t epollev) {
    int ev = EV_NONE;
    ev |= epollev & EPOLLOUT ? EV_WRITE : 0;
    ev |= epollev & EPOLLIN ? EV_READ : 0;
    ev |= epollev & EPOLLPRI ? EV_READ : 0;
    ev |= epollev & EPOLLERR ? EV_ERROR : 0;
    ev |= epollev & EPOLLHUP ? EV_ERROR : 0; // error, fd need remove
    ev |= epollev & EPOLLRDHUP ? EV_CLOSE : 0; // close 
    return ev;
}

EpollSelector::EpollSelector(EventDispatcher *evd):Selector(evd),_events(10) {
    if ((_epfd = ::epoll_create(0xbeef)) < 0) {
        LOG(FATAL) << "epoll creat fail";
    }
    LOG(TRACE) << "create epollfd = " << _epfd;
}

int EpollSelector::Add(int fd,int events) {
    struct epoll_event event;
    int evs = ((EV_READ & events) ? EPOLLIN : 0) | ((EV_WRITE & events) ? EPOLLOUT : 0);
    event.events = evs;
    event.data.fd = fd;	
    int ret = 0;
    LOG(TRACE) << " Add fd = " << fd << " events = " << evs;
    if ((ret = epoll_ctl(_epfd,EPOLL_CTL_ADD,fd,&event)) < 0) {
        LOG(FATAL) << "epoll add fail. fd = " << fd;
    }
    return ret;
}

int EpollSelector::Update(int fd,int events) {
    struct epoll_event event;
    int evs = ((EV_READ & events) ? EPOLLIN : 0) | ((EV_WRITE & events) ? EPOLLOUT : 0);
    LOG(TRACE) << "Update fd = " << fd << " events = " << evs;
    event.events = evs;
    event.data.fd = fd;	
    int ret = 0; 
    if ((ret = ::epoll_ctl(_epfd,EPOLL_CTL_MOD,fd,&event)) < 0) {
        LOG(FATAL) << "epoll mod fail . fd = " << fd;
    }
    return ret;
}

int EpollSelector::Remove(int fd) {
    struct epoll_event event;
    int ret = 0;
    if ((ret = ::epoll_ctl(_epfd,EPOLL_CTL_DEL,fd,&event)) < 0) {
        LOG(FATAL) << "epoll del fail. fd = " << fd;
    }
    return ret;
}

int EpollSelector::Select(int timeout,vector<Event> &events) {
    LOG(TRACE) << "Start select. _events.size = " << _events.size();
    const int n = ::epoll_wait(_epfd,&(*_events.begin()),_events.size(),timeout);
    LOG(TRACE) << "Select done. n = " << n;
    if (n > 0) {
        for (int i = 0; i < n; ++i) {
            Event ev = {_trans_epollev_to_ev(_events[i].events),_events[i].data.fd};
            events.push_back(ev);
            LOG(TRACE) << "event: fd = " << ev.fd << " revents = " << ev.revents;
        }
        if (static_cast<int>(_events.size()) == n) {
            LOG(TRACE) << "resize events.size to two times. size = " << 2*n;
            _events.resize(n*2);
        //} else if (static_cast<int>(_events.size()) > 2*n) {
        //    // FIXME need ? 
        //    LOG(TRACE) << "resize events.size to half. size = " << n/2;
        //    _events.resize(n/2);
        }
        return 0;
    } else if (0 == n) {
        LOG(TRACE) << "Nothing happend.";
        return 0;
    } else {
        // n < 0
        LOG(FATAL) << "Never arrive here! IO error. n = " << n;
        exit(-1);
        return -1;
    }
    return -1;
}
