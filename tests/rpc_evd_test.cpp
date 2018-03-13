#include <iostream>
#include <memory>

#include "rpc/timer/timer_queue.h"
#include "rpc/event_dispatcher.h"
#include "rpc/selector.h"
#include <functional>
#include "log/logging.h"
#include "rpc/timer/timer_queue_ll.h"

int main(int argc,char *argv[])
{
    yamq::initLogging(argv[0]);
    EventDispatcher evd;
    evd.AddTimer(1000,10,[](){printf("timer cb\n");});
    evd.Run();
    yamq::shutdownLogging();
    return 0;
}
