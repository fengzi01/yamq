#ifndef _EVENT_DISPATCHER_H
#define _EVENT_DISPATCHER_H
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>

using std::unique_ptr;
class Selector;
class TimerQueueRbtree;
class Channel;

class EventDispatcher {
    public:
        typedef std::function<void()> Functor;
        EventDispatcher();
        ~EventDispatcher();
        void Start();
        void Stop(); 

        // Never call in another thread!
        int RegisterChannel(Channel *);
        int UpdateChannel(Channel *);
        int RemoveChannel(Channel *);

        int AddTimer(int64_t time_ms,int interval,Functor cb);
        void CancelTimer(int timer_id);

        void Wakeup();
        void RunInLoop(const Functor &cb); 
    private:
        Channel* findChannel(int fd);
        void runPendingFunctor();
        void addPendingFunctor(const Functor &cb);

        bool isInLoopThread() const;

        std::atomic<bool> _stop;
        unique_ptr<Selector> _selector;  // for virtual function
        std::unordered_map<int,Channel *> _ref;

        unique_ptr<TimerQueueRbtree> _timer_queue;
        // FIXME or pipe ? 
        int _wakeup_fds[2];
        unique_ptr<Channel> _wakeup_channel;

        std::mutex _mutex;
        std::vector<Functor> _pending_functors;

        const int _thread_id;
};

#endif
