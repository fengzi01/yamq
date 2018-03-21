#include <iostream>
#include <memory>

#include "rpc/event_dispatcher.h"
#include "rpc/selector.h"
#include <functional>
#include "log/logging.h"
#include "rpc/timer/timer_queue_ll.h"
#include "rpc/timer/timer_utilties.h"

int main(int argc,char *argv[])
{
    yamq::initLogging(argv[0]);
    EventDispatcher evd;
    evd.AddTimer(1000,10000,[](){printf("timer 1 cb. now = %s \n",Clock::CurrentTimeString(Clock::CurrentTimeMillis()).c_str());});
    evd.AddTimer(2000,10000,[](){printf("timer 2 cb. now = %s \n",Clock::CurrentTimeString(Clock::CurrentTimeMillis()).c_str());});
    evd.AddTimer(3000,10000,[](){printf("timer 3 cb. now = %s \n",Clock::CurrentTimeString(Clock::CurrentTimeMillis()).c_str());});
    evd.Run();
    yamq::shutdownLogging();
    return 0;
}
