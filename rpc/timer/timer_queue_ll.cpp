// Copyright (C) 2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "rpc/timer/timer_queue_ll.h"
#include "rpc/timer/timer_utilties.h"
#include "log/logging.h"
#include "rpc/event_dispatcher.h"

#include "rpc/timer/timerfd.h"
#include <algorithm>

TimerQueue_linked_list::TimerQueue_linked_list(EventDispatcher *evd)
    :twepoch(Clock::GetNowTicks())
{
    _ref.rehash(16);
    _fd = createTimerfd();
    LOG(TRACE) << "create timerfd = " << _fd;
    _evd = evd;
}

TimerQueue_linked_list::~TimerQueue_linked_list()
{
    clear();
    closeTimerfd(_fd);
}

void TimerQueue_linked_list::Start() {
    SetEvents(EV_READ);
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
    
    int id = nextId();

    timer->id = id;
    timer->expire = expire;
    timer->cb = cb;
    timer->index = -1;
    timer->elapse = interval;
    timer->interval = interval;
    timer->time = time;

    LOG(TRACE) << "Add Timer. id = " << timer->id << " expire = " << timer->expire / 1000000UL << " ms";

    _ref[timer->id] = timer;
    if ( _linked_list.empty() || _linked_list.back()->expire > expire) {
        _linked_list.push_back(timer);

        LOG(TRACE) << "_linked_list.back() " << _linked_list.back()->id << " expire = " << _linked_list.back()->expire/1000000UL;
        // reset timerfd
        resetTimerfd(this->_fd,twepoch + expire);
    } else {
        _linked_list.push_back(timer);
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
    if (nullptr == node) {
        return false;
    }

    if (node->expire <= _linked_list.back()->expire) {
        // back
        _linked_list.pop_back();
    }

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

int64_t TimerQueue_linked_list::WaitTimeUsec() {
    LOG(TRACE) << "_linked_list.empty() " << _linked_list.empty() << " size = " << _linked_list.size();
    LOG(TRACE) << "========================";
    LOG(TRACE) << "SIZE = " << _linked_list.size();
    for ( auto node : _linked_list) {
        LOG(TRACE) << node->expire/1000000UL;
    }
    LOG(TRACE) << "========================";
    if (!_linked_list.empty()) {
        LOG(TRACE) << "expire = " << _free_list[0]->expire / 1000000UL;
        return _free_list.back()->expire; 
    }
    return -1;
}

void TimerQueue_linked_list::PerTick()
{
    uint64_t now = Clock::GetNowTicks() - twepoch;
    std::vector<Timer *> expired;
    LOG(TRACE) << "========================";
    LOG(TRACE) << "SIZE = " << _linked_list.size();
    for ( auto node : _linked_list) {
        LOG(TRACE) << node->expire/1000000UL;
    }
    LOG(TRACE) << "NOW = " << now / 1000000UL;
    LOG(TRACE) << "========================";
    while (!_linked_list.empty())
    {
        if (now < _linked_list.back()->expire)
        {
            LOG(TRACE) << " now is not expired";
            break;
        }
        auto node = _linked_list.back();
        expired.push_back(node);
        _linked_list.pop_back();
    }

    for ( auto node : expired) {
        if (node->elapse > 0) {
            --(node->elapse);
            node->cb();
        }
        if (node->elapse > 0) {
            // FIXME
            LOG(TRACE) << "ADD TIMER " << node->id << " ela = " << node->elapse;
            AddTimer(node->time,node->elapse,node->cb);
        }
        _free_list.push_back(node);
    }
}

void TimerQueue_linked_list::HandleRead() {
    LOG(TRACE) << "_LINK_LIST " << _linked_list.size();
    LOG(TRACE) << _linked_list.back()->id;
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
