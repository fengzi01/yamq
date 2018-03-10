#ifndef _TIMER_H
#define _TIMER_H
#include <functional>
#include <memory>
#include "channel.h"
using std::unique_ptr;

class Timer {
    public:
        Timer(int timeout,std::function<void()> &&callback);
        int time();
    private:
        std::function<void()> _callback;
};

class TimerHeap {
    public:
        unique_ptr<Timer> top();
};
#endif
