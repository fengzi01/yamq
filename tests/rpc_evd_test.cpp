#include <iostream>
#include <memory>

#include "rpc/timer/timer_queue.h"
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
    evd.AddTimer(1000,10000,[](){printf("timer cb. now = %s \n",Clock::CurrentTimeString(Clock::CurrentTimeMillis()).c_str());});
    evd.Run();
    yamq::shutdownLogging();
    return 0;
}
