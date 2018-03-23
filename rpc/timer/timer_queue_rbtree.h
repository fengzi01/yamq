#include "rpc/timer/rbtree/rbtree.h"
#include "rpc/timer/timer_queue_base.h"
#include "rpc/channel.h"

/**
 * 定时器队列红黑树实现
 */
class TimerQueueRbtree : public TimerQueueBase,public Channel {
    public:
        TimerQueueRbtree(EventDispatcher *evd);
        virtual ~TimerQueueRbtree();

        // thread safe
        int AddTimer(uint64_t time,uint64_t interval,TimerCallback cb) override;
        // thread safe
        void CancelTimer(int id) override;
        void PerTick() override;
        int Size() const override { return -1; }
        virtual int64_t WaitTimeUsec() const override;
//        void Start() override;
        virtual void HandleRead() override;
    private:
        void addTimer(Timer *timer);
        bool cancelTimer(int timer_id);
        rbtree_t *_tree;
        const uint64_t _own_epoch;
};

