#ifndef _EVENT_DISPATCHER_H
#define _EVENT_DISPATCHER_H
#include <memory>
#include <map>
#include "rpc/channel.h"
#include "rpc/timer.h"

using std::unique_ptr;
using std::map;
class Selector;

class EventDispatcher {
    public:
        EventDispatcher();
        void Run();
        int RegisterChannel(Channel *);
        int UpdateChannel(Channel *);
        int RemoveChannel(Channel *);
        Channel* FindChannel(int fd);
    private:
        bool _stop;
        unique_ptr<Selector> _selector;  // for virtual function
        map<int,Channel *> _fd_channel_map;
};

#endif
