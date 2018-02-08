#ifndef YAMQ_UTILITIES
#define YAMQ_UTILITIES 
#include <time.h> // gettimeofday
#include <sys/time.h> // timeval
#include <stdint.h>
//#include <sys/types.h> // pid_t
#include <unistd.h> // getpid
#include <string.h>

namespace yamq {
typedef uint64_t Timestamp; // seconds * 1000000 + microseconds

typedef uint32_t UnixTimestamp;
typedef struct tm ReadableTime;
typedef pid_t Tid;

/* 当前时间 */
Timestamp nowTime(ReadableTime *tm);
ReadableTime localTime();

/* 获取当前进程id */
Tid getTid();

} // yamq
#endif /* ifndef YAMQ_UTILITIES */
