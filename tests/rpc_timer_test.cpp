#include <iostream>
#include <memory>

#include "rpc/event_dispatcher.h"
#include "rpc/selector.h"
#include <functional>
#include "log/logging.h"
#include "rpc/timer/timer_queue_ll.h"

TimerQueue_linked_list *queue;

void TimerHandler()
{
    std::cout << " >>> TimerHandler" << std::endl;
}

int main(int argc,char *argv[])
{
    yamq::initLogging(argv[0]);
    EventDispatcher evd;
    queue = new TimerQueue_linked_list(&evd);
    queue->AddTimer(1000000,10,TimerHandler);
//    queue->Start();
    evd.Run();
    delete queue;
    yamq::shutdownLogging();
    return 0;
}
