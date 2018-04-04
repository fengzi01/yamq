//#include <time.h> // gettimeofday
//#include <sys/time.h> // timeval
#include "utilities.h"
namespace yamq{ 
/* 获取当前时间 */
Timestamp nowTime(ReadableTime *tm) {
	// TODO(hamaji): temporary impementation - it might be too slow.
	struct timeval tv;
    if (NULL != tm) {
        struct timezone tz;
        ::gettimeofday(&tv, &tz);
        // 转换为本地可读时间
        memset(tm,0, sizeof(ReadableTime));
        time_t t = 0;
        // FIXME
        assert(tz.tz_minuteswest == -480);
        // +偏移分钟数
        t = tv.tv_sec - tz.tz_minuteswest*60;
        ::gmtime_r(&t,tm);
        tm->tm_gmtoff = tz.tz_minuteswest * 60;
        tm->tm_zone = NULL;
//        fprintf(stderr,"off:%d\n",tz.tz_minuteswest);
    } else {
        ::gettimeofday(&tv,NULL);
    }
	return static_cast<Timestamp>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

Tid getTid() {
    return static_cast<Tid>(getpid());
}

char g_projectName[256];
char g_projectDirname[1024];

const char *getProjectName() {
    return g_projectName;
}
const char *getProjectDirname() {
    return g_projectDirname;
}
bool initUtilities(const char *argv0) {
    assert(NULL!=argv0);
    strncpy(g_projectDirname,argv0,sizeof(g_projectDirname));

    char* slash = strrchr(g_projectDirname, '/');
    strncpy(g_projectName,slash+1,sizeof(g_projectName));
    *(++slash) = '\0';
    return true;
}
}
