// Copyright (C) 2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "rpc/timer/timer_queue.h"
#include "rpc/timer/timer_utilties.h"
#include "log/logging.h"
#include "rpc/event_dispatcher.h"
//#include <unistd.h>
#include "rpc/timer/timerfd.h"

/* timerfd */

struct Timer
{
    int index = -1;
    int id = -1;
    int64_t expires = 0;
    TimerCallback cb;

#define HEAP_ITEM_LESS(i, j) (heap_[(i)]->expires < heap_[(j)]->expires)

};

TimerQueue::TimerQueue(EventDispatcher *evd)
    :twepoch(Clock::CurrentTimeMillis()) 
{
    _ref.rehash(16);

    _fd = createTimerfd();
    LOG(TRACE) << "timerfd = " << _fd;
    _evd = evd;

    heap_.reserve(16);
}

TimerQueue::~TimerQueue()
{
    clear();
    closeTimerfd(_fd);
}

void TimerQueue::Start() {
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

void TimerQueue::OnRead() {
    uint64_t now = Clock::GetNowTicks();
    readTimerfd(_fd,now);
    PerTick();
    resetTimerfd(_fd,now + 1000000UL*100);
}

void TimerQueue::clear()
{
    for (auto ptr : heap_)
    {
        delete ptr;
    }
    heap_.clear();
    _ref.clear();
}

bool TimerQueue::siftdown(int x, int n)
{
    int i = x;
    for (;;)
    {
        int j1 = 2 * i + 1;
        if ((j1 >= n) || (j1 < 0)) // j1 < 0 after int overflow
        {
            break;
        }
        int j = j1; // left child
        int j2 = j1 + 1;
        if (j2 < n && !HEAP_ITEM_LESS(j1, j2))
        {
            j = j2; // right child
        }
        if (!HEAP_ITEM_LESS(j, i))
        {
            break;
        }
        std::swap(heap_[i], heap_[j]);
        heap_[i]->index = i;
        heap_[j]->index = j;
        i = j;
    }
    return i > x;
}

void TimerQueue::siftup(int j)
{
    for (;;)
    {
        int i = (j - 1) / 2; // parent node
        if (i == j || !HEAP_ITEM_LESS(j, i))
        {
            break;
        }
        std::swap(heap_[i], heap_[j]);
        heap_[i]->index = i;
        heap_[j]->index = j;
        j = i;
    }
}

int TimerQueue::AddTimer(uint64_t time,uint64_t interval, TimerCallback cb)
{
    int64_t expire = Clock::CurrentTimeMillis() - twepoch + time;
    Timer* node = new Timer;

    node->id = nextSlot();
    node->expires = expire;
    node->cb = cb;
    node->index = heap_.size();

    heap_.push_back(node);
    siftup(heap_.size() - 1);
    _ref[node->id] = node;
    LOG(TRACE) << "Add Timer. id = " << node->id << " expire = " << node->expires;
    return node->id;
}

bool TimerQueue::CancelTimer(int id)
{
    Timer* node = (Timer*)_ref[id];
    if (node != nullptr)
    {
        int n = (int)heap_.size() - 1;
        int i = node->index;
        if (i != n)
        {
            std::swap(heap_[i], heap_[n]);
            heap_[i]->index = i;
            if (!siftdown(i, n))
            {
                siftup(i);
            }
        }
        heap_.pop_back();
        _ref.erase(id);
        delete node;
        return true;
    }
    return false;
}

void TimerQueue::PerTick()
{
    int64_t now = Clock::CurrentTimeMillis() - twepoch;
    while (!heap_.empty())
    {
        Timer* node = heap_.front();
        if (now < node->expires)
        {
            break;
        }
        int n = (int)heap_.size() - 1;
        std::swap(heap_[0], heap_[n]);
        heap_[0]->index = 0;
        siftdown(0, n);
        heap_.pop_back();
        if (node->cb)
        {
            node->cb();
        }
        _ref.erase(node->id);
        delete node;
    }
}
