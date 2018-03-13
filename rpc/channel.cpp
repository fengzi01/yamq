#include "rpc/channel.h"
#include "rpc/event_dispatcher.h"
#include "log/logging.h"

void Channel::HandleEvent(Event &ev) { 
    LOG(TRACE) << "Handle event, fd = " << ev.fd << ", ev = " << ev.revents;
    if ( ev.revents == EV_NONE) {
        return;
    }
    if ( ev.revents & EV_READ) {
        OnRead();
    }
    if (ev.revents & EV_WRITE) {
        OnWrite();
    }
    if (ev.revents & EV_ERROR) {
        OnError();
    }
} 

void Channel::Remove() {
    _evd->RemoveChannel(this);
}
void Channel::Update() {
    if (_attached) {
        _evd->UpdateChannel(this);
    } else {
        _evd->RegisterChannel(this);
        _attached = true;
    }
}

