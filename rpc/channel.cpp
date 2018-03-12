#include "rpc/channel.h"
#include "rpc/event_dispatcher.h"

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
