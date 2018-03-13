#include <iostream>
#include <thread>
#include <chrono>
#include "rpc/timer/timer_queue.h"

TimerQueue queue;

void TimerHandler()
{
    std::cout << "TimerHandler" << std::endl;
    queue.AddTimer(1000,TimerHandler);
}

int main()
{
    queue.AddTimer(1000,TimerHandler);
    while (true)
    {
        queue.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cin.get();
    return 0;
}
