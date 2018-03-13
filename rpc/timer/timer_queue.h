// Copyright (C) 2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <sys/timerfd.h>
#include <list>
#include <vector>
#include <memory>
#include <unordered_map>
#include "rpc/timer/timer_queue_base.h"
#include "rpc/channel.h"


struct Timer;

// timer queue implemented with priority queue(min-heap)
//
// complexity:
//     AddTimer    CancelTimer   PerTick
//      O(log N)    O(log N)       O(1)
//
class TimerQueue : public TimerQueueBase, public Channel
{
public:
    TimerQueue(EventDispatcher *evd);
    ~TimerQueue();

    int AddTimer(uint64_t time,uint64_t interval,TimerCallback cb) override;

    void Start();

    bool CancelTimer(int id) override;

    void PerTick() override;

    int Size() const { return (int)heap_.size(); }

    virtual void OnRead() override;

private:
    void clear();
    bool siftdown(int x, int n);
    void siftup(int j);

private:
    const int64_t twepoch; // custom epoch
    std::vector<Timer*> heap_;
};

class TimerQueue_linked_list : public TimerQueueBase, public Channel {
    TimerQueue_linked_list(EventDispatcher *evd);
    virtual ~TimerQueue_linked_list();

    int AddTimer(uint64_t time,uint64_t interval,TimerCallback cb) override;
    bool CancelTimer(int id) override;
    void PerTick() override;
    int Size() const override { return _linked_list.size(); }
    virtual int64_t WaitTimeUsec() override {}
    virtual void OnRead() override;

    void Start();

private:
    void clear();
    bool siftdown(int x, int n);
    void siftup(int j);

private:
    const int64_t twepoch; // custom epoch
    std::list<std::unique_ptr<Timer>> _linked_list;
};
