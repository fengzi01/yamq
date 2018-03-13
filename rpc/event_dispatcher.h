#ifndef _EVENT_DISPATCHER_H
#define _EVENT_DISPATCHER_H
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "rpc/channel.h"
#include "rpc/timer.h"

using std::unique_ptr;
class Selector;

class EventDispatcher {
    public:
        typedef std::function<void()> Functor;
        EventDispatcher();
        void Run();
        void Stop(); 

        int RegisterChannel(Channel *);
        int UpdateChannel(Channel *);
        int RemoveChannel(Channel *);
        Channel* FindChannel(int fd);

        void AddPendingFunctor(Functor &&cb);
        void Wakeup();
    private:
        void runPendingFunctor();
        // FIXME 原子操作?
        bool _stop;
        unique_ptr<Selector> _selector;  // for virtual function
        std::unordered_map<int,Channel *> _fd_channel_map;

        std::mutex _mutex;
        std::vector<Functor> _pending_functors;

        // FIXME or pipe ? 
        int _wakeup_fds[2];
        unique_ptr<Channel> _wakeup_channel;
};

#endif
