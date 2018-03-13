#ifndef _RPC_TIMERFD_H
#define _RPC_TIMERFD_H
#include <unistd.h>
#include <sys/timerfd.h>
#include "log/logging.h"
#include "rpc/timer/timer_utilties.h"

/* timerfd */
int createTimerfd(); 

void resetTimerfd(int fd,uint64_t ticks);

void readTimerfd(int fd,int64_t now); 

void closeTimerfd(int fd); 
#endif
