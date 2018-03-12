#include "rpc/event_dispatcher.h"
#include "rpc/selector.h"
#include "rpc/channel.h"
#include "assert.h"
#include "log/logging.h"

EventDispatcher::EventDispatcher():_stop(false),_selector(MakeDefaultSelector(this)) {}

void EventDispatcher::Run() {
    vector<Event> events;
    // loop
    for(;;) {
        events.clear();
        _selector->Select(1000,events);
        LOG(TRACE) << "events.size = " << events.size();
        for (auto it = events.begin(); it != events.end(); ++it) {
            Channel *channel = FindChannel(it->fd);
            if (nullptr != channel) {
                LOG(TRACE) << "get a channel = " << channel << ",fd = " << it->fd;
                channel->HandleEvent(*it);
            } else {
                LOG(WARNING) << "Found fd which don't have Channel.fd = " << it->fd;
                exit(-1); 
            }
        } if (_stop) {
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
