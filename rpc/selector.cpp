#include <algorithm>
#include "rpc/selector.h"
#include "rpc/event_dispatcher.h"
#include "log/logging.h"
#include <assert.h>

int PollSelector::Select(int timeout,vector<Event> &events) {
    int active;
    LOG(TRACE) << "poll ...";
    active = ::poll(&(*_pollfds.begin()),_pollfds.size(),timeout);
    if (active >= 0) {
        if (0 == active) {
            LOG(TRACE) << "Nothing happned. pollfds.size:" << _pollfds.size();
            return -1;
        }
        LOG(TRACE) << "active events = " << active;
        //Channel *channel = nullptr;
        for (auto pfd = _pollfds.begin(); pfd != _pollfds.end() && active > 0; ++pfd) {
            if (pfd->revents > 0) {
                --active;
                //channel = _evd->FindChannel(pfd->fd);
                // FIXME Event type ?
                Event event = {pfd->revents,pfd->fd};
                events.push_back(event); 
            }
        }
    } else {
        // Io error
        LOG(FATAL) << "Io Error!";
        return -2;
    }
    return 0;
}

int PollSelector::Add(int fd,int events) {
    LOG(INFO) << "events = " << events << ",POLLIN = " << POLLIN << ",POLLOUT = " << POLLOUT;
    short evs = ((EV_READ & events) ? POLLIN : 0) | ((EV_WRITE & events) ? POLLOUT : 0);
    pollfd pfd{fd,evs,0};
    _pollfds.push_back(pfd);
    LOG(INFO) << "Add fd = " << pfd.fd << ",evs = "<< pfd.events;
    LOG(TRACE) << "pollfds.size = " << _pollfds.size();
    _fd_idx_map[fd] = _pollfds.size()-1;
    return 0;
}

int PollSelector::Update(int fd,int events) {
    size_t idx;
    auto it = _fd_idx_map.find(fd);
    if (it == _fd_idx_map.end()) {return -1;}
    short evs = ((EV_READ & events) ? POLLIN : 0) | ((EV_WRITE & events) ? POLLOUT : 0);
    idx = it->second;
    _pollfds[idx].events = evs;
    return 0;
}

int PollSelector::Remove(int fd) {
    size_t idx;
    auto it = _fd_idx_map.find(fd);
    if (it == _fd_idx_map.end()) {return -1;}
    idx = it->second;

    LOG(TRACE) << "Remove fd = " << it->first << " ,idx = " << it->second;

    if (idx == _fd_idx_map.size()-1) {
        _pollfds.pop_back();
    } else {
        // 待删除fd与最后一个元素交换
        LOG(TRACE) << "_pollfds.back().fd = " << _pollfds.back().fd << ", fd = " << fd;
        _fd_idx_map[_pollfds.back().fd] = idx;
        std::iter_swap(_pollfds.end()-1,_pollfds.begin()+idx);
        _pollfds.pop_back();
    }
    _fd_idx_map.erase(fd);
    return 0;
}

unique_ptr<Selector> MakeDefaultSelector(EventDispatcher *evd) {
    return unique_ptr<Selector>(new PollSelector(evd));
};
