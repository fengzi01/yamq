// Copyright (C) 2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <stdint.h>
#include <functional>
#include <unordered_map>

// callback on timed-out
typedef std::function<void()> TimerCallback;

// timer queue scheduling
class TimerQueueBase
{
public:
    TimerQueueBase() {}
    ~TimerQueueBase() {}

    TimerQueueBase(const TimerQueueBase&) = delete;
    TimerQueueBase& operator=(const TimerQueueBase&) = delete;

    // add a timer to schedule after `time` milliseconds.
    // returns an unique id identify this timer.
    virtual int AddTimer(uint32_t time, TimerCallback cb) = 0;

    // cancel a timer by id.
    virtual bool CancelTimer(int id) = 0;

    // per-tick bookkeeping.
    virtual void Update() = 0;

    virtual int Size() const = 0;

protected:
    virtual void getReadyTimer(long long now) {}

    int nextSlot();

    // FIXME
    int _slot = 0;
    std::unordered_map<int, void*> _ref; // keep a hash reference to id, make O(1) searching
};
