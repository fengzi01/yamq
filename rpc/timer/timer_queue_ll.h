// 定时器链表实现
#pragma once

#include <list> // 双向链表
#include <vector>
#include <memory>
#include <unordered_map>
#include "rpc/timer/timer_queue_base.h"
#include "rpc/channel.h"


struct Timer;

class TimerQueue_linked_list : public TimerQueueBase, public Channel {
    public:
        TimerQueue_linked_list(EventDispatcher *evd);
        virtual ~TimerQueue_linked_list();

        int AddTimer(uint64_t time,uint64_t interval,TimerCallback cb) override;
        bool CancelTimer(int id) override;
        void PerTick() override;
        int Size() const override { return -1; }
        virtual int64_t WaitTimeUsec() override;
        virtual void OnRead() override;

        void Start() override;

private:
    void clear();

private:
    const int64_t twepoch; // custom epoch
    std::vector<Timer*> _linked_list;
    std::vector<Timer*> _free_list;
};
