#include "rpc/channel.h"
#include "rpc/event_dispatcher.h"

void Channel::Update() {
    _evd->UpdateChannel(this);
}
