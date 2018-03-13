#include "rpc/timer/timerfd.h"
/* timerfd */
int createTimerfd() {
  //int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
  //                               TFD_NONBLOCK | TFD_CLOEXEC);
  int timerfd = ::timerfd_create(CLOCK_REALTIME,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0)
  {
    LOG(FATAL) << "Failed in timerfd_create";
  }
  return timerfd;
}

void resetTimerfd(int fd,uint64_t ticks) {
    // wake up loop by timerfd_settime()
    struct itimerspec new_val;
    struct itimerspec old_val;
    bzero(&new_val, sizeof new_val);
    bzero(&old_val, sizeof old_val);
    new_val.it_value = Clock::TransTicks(ticks);
    struct timespec now = Clock::TransTicks(Clock::GetNowTicks());
    LOG(TRACE) << "now.sec = "  << now.tv_sec << " now.nsec = " << now.tv_nsec;
    LOG(TRACE) << "resetfd_settime" << " val.sec = " << new_val.it_value.tv_sec
        << " val.nsec = " << new_val.it_value.tv_nsec;
    int ret = ::timerfd_settime(fd, 1, &new_val, &old_val); // IMPORTANT !!! flag = 1
    LOG(TRACE) << " timerfd_settime ret = " << ret;
    if (0 != ret)
    {
        LOG(FATAL) << "timerfd_settime()";
    }
}

void readTimerfd(int fd,int64_t now) {
  uint64_t howmany;
  ssize_t n = ::read(fd, &howmany, sizeof howmany);
  LOG(TRACE) << "TimerQueue::handleRead() " << howmany << " at " << Clock::CurrentTimeString(now / 1000000UL);
  if (n != sizeof howmany)
  {
    LOG(FATAL) << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
  }
}

void closeTimerfd(int fd) {
    ::close(fd);
}
