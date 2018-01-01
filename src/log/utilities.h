#ifndef _LOG_UTILITIES
#define _LOG_UTILITIES 
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

typedef long long int64;
typedef double WallTime;

int64 CycleClock_Now() {
    // TODO(hamaji): temporary impementation - it might be too slow.
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast<int64>(tv.tv_sec) * 1000000 + tv.tv_usec;
}
WallTime WallTime_Now() {
    // Now, cycle clock is retuning microseconds since the epoch.
    return CycleClock_Now() * 0.000001;
}
const char* const_basename(const char* filepath) {
    const char* base = strrchr(filepath, '/');
#ifdef OS_WINDOWS  // Look for either path separator in Windows
    if (!base)
        base = strrchr(filepath, '\\');
#endif
    return base ? (base+1) : filepath;
}
#endif /* ifndef _LOG_UTILITIES */
