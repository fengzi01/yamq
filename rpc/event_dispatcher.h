#ifndef _EVENT_DISPATCHER_H
#define _EVENT_DISPATCHER_H
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "rpc/channel.h"
#include "rpc/timer/timer_queue_base.h"
#include <atomic>

using std::unique_ptr;
class Selector;

class EventDispatcher {
    public:
        typedef std::function<void()> Functor;
        EventDispatcher();
        ~EventDispatcher();
        void Run();
        void Stop(); 

        // Never call in another thread!
        int RegisterChannel(Channel *);
        int UpdateChannel(Channel *);
        int RemoveChannel(Channel *);

        void AddPendingFunctor(Functor &&cb);
        void Wakeup();

        // Never call in another thread!
        int AddTimer(int64_t time_ms,int interval,Functor cb);
        void CancelTimer(int timer_id);
    private:
        Channel* findChannel(int fd);
        void runPendingFunctor();
        std::atomic<bool> _stop;
        unique_ptr<Selector> _selector;  // for virtual function
        std::unordered_map<int,Channel *> _ref;

        std::mutex _mutex;
        std::vector<Functor> _pending_functors;

        // FIXME or pipe ? 
        int _wakeup_fds[2];
        unique_ptr<Channel> _wakeup_channel;

        unique_ptr<TimerQueueBase> _timer_queue;
};

#endif
