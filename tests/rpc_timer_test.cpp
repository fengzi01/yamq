#include <iostream>
#include <memory>

#include "rpc/timer/timer_queue.h"
#include "rpc/event_dispatcher.h"
#include "rpc/selector.h"
#include <functional>
#include "log/logging.h"
#include "rpc/timer/timer_queue_ll.h"

TimerQueue_linked_list *queue;

void TimerHandler()
{
    std::cout << " >>> TimerHandler" << std::endl;
    queue->AddTimer(1000,1,TimerHandler);
}

int main(int argc,char *argv[])
{
    yamq::initLogging(argv[0]);
    EventDispatcher evd;
    queue = new TimerQueue_linked_list(&evd);
    queue->SetEvents(EV_READ|EV_WRITE);
    queue->AddTimer(1000,1,TimerHandler);
    queue->Start();
    evd.Run();
    delete queue;
    yamq::shutdownLogging();
    return 0;
}
