//#include <time.h> // gettimeofday
//#include <sys/time.h> // timeval
#include "utilities.h"
namespace yamq{ 
/* 获取当前时间 */
Timestamp nowTime() {
	// TODO(hamaji): temporary impementation - it might be too slow.
	struct timeval tv;
	::gettimeofday(&tv, NULL);
	return static_cast<Timestamp>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

Tid getTid() {
    return static_cast<Tid>(getpid());
}
}
