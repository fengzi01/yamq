#include "rpc/event_dispatcher.h"
#include "rpc/selector.h"
#include "rpc/channel.h"
#include "assert.h"

EventDispatcher::EventDispatcher():_stop(false),_selector(MakeDefaultSelector()) {
}

void EventDispatcher::Run() {
    vector<Event> events;
    // loop
    for(;;) {
        events.clear();
        _selector->Select(100,events);
        for (auto it = events.begin(); it != events.end(); ++it) {
            it->channel->HandleEvent(*it);
        } if (_stop) {
            break;
        }
    }
}

int EventDispatcher::RegisterChannel(Channel *channel) {
    int fd = channel->Getfd();
    _selector->Add(fd,channel->GetRevents());
    _channel_map.insert(std::pair<int,Channel *>(fd,channel));
    return 0;
}

int EventDispatcher::UpdateChannel(Channel *channel) {
    _selector->Update(channel->Getfd(),channel->GetRevents()); 
}

int EventDispatcher::RemoveChannel(Channel *channel) {
    auto it = _channel_map.find(channel->Getfd());
    assert(it->second == channel);

    int fd = channel->Getfd();
    _selector->Remove(fd);
    _channel_map.erase(fd);
};

Channel *EventDispatcher::FindChannel(int fd) {
    auto it = _channel_map.find(fd);

    if (it == _channel_map.end()) {
        return nullptr;
    }
    return it->second;
}
