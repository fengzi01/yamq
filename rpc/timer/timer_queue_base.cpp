// Copyright (C) 2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "rpc/timer/timer_queue_base.h"

int TimerQueueBase::nextSlot()
{
    int next = _slot + 1;
    for (;;)
    {
        next = next < 0 ? 0 : next;
        if (_ref.count(next) > 0)
        {
            ++next;
            continue;
        }
        break;
    }
    _slot = next;
    return next;
}
