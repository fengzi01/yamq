// Copyright (C) 2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "rpc/timer/timer_queue_ll.h"
#include "rpc/timer/timer_utilties.h"
#include "log/logging.h"
#include "rpc/event_dispatcher.h"

#include "rpc/timer/timerfd.h"
#include <algorithm>

struct Timer
{
    int index = -1;
    int id = -1;
    uint64_t expire = 0;
    int64_t interval = -1; // -1 重复 0 不重复 n 重复次数
    int64_t elapse = -1;
    TimerCallback cb;
};

TimerQueue_linked_list::TimerQueue_linked_list(EventDispatcher *evd)
    :twepoch(Clock::GetNowTicks()), _linked_list(10)
{
    _ref.rehash(16);
    _fd = createTimerfd();
    LOG(TRACE) << "timerfd = " << _fd;
    _evd = evd;
}

TimerQueue_linked_list::~TimerQueue_linked_list()
{
    clear();
    closeTimerfd(_fd);
}

void TimerQueue_linked_list::Start() {
    if (_evd) {
        LOG(TRACE) << "Add functor in evd !";
        _evd->AddPendingFunctor(
            [this]() {
                LOG(TRACE) << "resetTimerfd IN EVD";
                uint64_t now = Clock::GetNowTicks();
                resetTimerfd(this->_fd,now + 1000000UL);
            }
        );
    }
}


void TimerQueue_linked_list::clear()
{
    _linked_list.clear();
    _free_list.clear();
    _ref.clear();
}

int TimerQueue_linked_list::AddTimer(uint64_t time,uint64_t interval, TimerCallback cb)
{
    uint64_t now = Clock::GetNowTicks();
    uint64_t expire = now - twepoch + time;
    Timer *timer;
    if (!_free_list.empty()) {
        timer = _free_list.back();
        _free_list.pop_back();
    } else {
        timer = new Timer();
    }
    
    int id = nextSlot();

    timer->id = id;
    timer->expire = expire;
    timer->cb = cb;
    timer->index = -1;
    timer->elapse = interval;
    timer->interval = interval;

    LOG(TRACE) << "Add Timer. id = " << timer->id << " expire = " << timer->expire / 1000000UL << "ms";

    _ref[timer->id] = timer;
    if ( _linked_list.empty() || _linked_list.back()->expire > expire) {
        _linked_list.push_back(std::move(timer));
        // reset timerfd
        resetTimerfd(this->_fd,twepoch + expire);
    } else {
        _linked_list.push_back(std::move(timer));
        std::sort(_linked_list.begin(), _linked_list.end(), 
            [](Timer *lhs,   Timer *rhs) {
                return  lhs->expire > rhs->expire;
            }
        );
    }
    return id;
}

bool TimerQueue_linked_list::CancelTimer(int id)
{
    Timer* node = (Timer*)_ref[id];
    if (node != nullptr)
    {
        std::swap(_linked_list.back(),node);
        _linked_list.pop_back();
        _ref.erase(id);

        std::sort(_linked_list.begin(), _linked_list.end(), 
            [](Timer *lhs,   Timer *rhs) {
                return  lhs->expire > rhs->expire;
            }
        );

        _free_list.push_back(node);
        return true;
    }
    return false;
}

int64_t TimerQueue_linked_list::WaitTimeUsec() {
    if (!_linked_list.empty()) {
        return _free_list.back()->expire; 
    }
    return -1;
}

void TimerQueue_linked_list::PerTick()
{
    uint64_t now = Clock::GetNowTicks() - twepoch;
    while (!_linked_list.empty())
    {
        if (now < _linked_list.back()->expire)
        {
            break;
        }
        auto node = _linked_list.back();
        if (node->cb)
        {
            --node->elapse; 
            node->cb();
        } else {
            // 没有执行意义
            node->elapse = -1;
        }
        if (node->elapse < 0) {
            _ref.erase(node->id);
            _free_list.push_back(node);
        } else {
            _linked_list.push_back(node);
        }
    }
}

void TimerQueue_linked_list::OnRead() {
    uint64_t now = Clock::GetNowTicks();
    readTimerfd(_fd,now);
    PerTick();
    int64_t expire = WaitTimeUsec();
    if (expire > 0) {
        resetTimerfd(_fd,twepoch + expire);
    } else {
        // FIXME
    }
}
