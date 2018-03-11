#include <algorithm>
#include "rpc/selector.h"
#include "rpc/event_dispatcher.h"
#include "log/logging.h"

int PollSelector::Select(int timeout,vector<Event> &events) {
    int active;
    active = ::poll(&(*_pollfds.begin()),_pollfds.size(),timeout);
    if (active >= 0) {
        if (0 == active) {
            LOG(TRACE) << "Nothing happned. pollfds.size:" << _pollfds.size();
            LOG(TRACE) << "fd = " << _pollfds[0].fd << "," << _pollfds[0].revents << "," << _pollfds[0].events;
            return -1;
        }
        Channel *channel = nullptr;
        for (auto pfd = _pollfds.begin(); pfd != _pollfds.end() && active > 0; ++pfd) {
            if (pfd->revents > 0) {
                --active;
                channel = _dispatcher->FindChannel(pfd->fd);
                // FIXME Assert channel == nullptr
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
    LOG(INFO) << "events = " << events << ",POLLIN = " << POLLIN << ",POLLOUT " << POLLOUT;
    //short evs = ((EV_READ & events) | POLLIN) | ((EV_WRITE & events) | POLLOUT);
    short evs = POLLIN | POLLOUT;
    pollfd pfd{fd,0,evs};
    _pollfds.push_back(pfd);
    LOG(TRACE) << "fd = " << _pollfds[0].fd << "," << _pollfds[0].revents << "," << _pollfds[0].events;
    LOG(INFO) << "Add fd = " << pfd.fd << ",evs = "<< pfd.revents;
    _fd_idx_map.insert(std::pair<int,size_t>(fd,_pollfds.size()-1));
    return 0;
}

int PollSelector::Update(int fd,int events) {
    size_t idx;
    auto it = _fd_idx_map.find(fd);
    if (it == _fd_idx_map.end()) {return -1;}
    short evs = ((EV_READ & events) & POLLIN) | ((EV_WRITE & events) & POLLOUT);
    idx = it->second;
    _pollfds[idx].events = evs;
    return 0;
}

int PollSelector::Remove(int fd) {
    size_t idx;
    auto it = _fd_idx_map.find(fd);
    if (it == _fd_idx_map.end()) {return -1;}
    idx = it->second;

    std::iter_swap(_pollfds.end()-1,_pollfds.begin()+idx);
    _pollfds.pop_back();
    _fd_idx_map.erase(fd);
    return 0;
}

unique_ptr<Selector> MakeDefaultSelector() {
    return unique_ptr<Selector>(new PollSelector);
};
