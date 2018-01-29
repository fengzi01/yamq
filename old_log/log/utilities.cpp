#include "log/utilities.h"
#include "stdio.h"

static const char *g_program_short_name = NULL;

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

void InitLogUtilities(const char *argv0) {
    if (g_program_short_name != NULL) {
        fprintf(stderr,"You called InitLog twice!\n");
        return;
    }

    const char* slash = strrchr(argv0, '/');
#ifdef OS_WINDOWS
    if (!slash)  slash = strrchr(argv0, '\\');
#endif
    g_program_short_name = slash ? slash + 1 : argv0;
}

void ShutdownLogUtilities() {
    if (NULL == g_program_short_name) {
        fprintf(stderr,"You called ShutdownLog without calling InitLog\n");
        return;
    }

    g_program_short_name = NULL;
}

static int g_main_thread_pid = getpid();
int GetMainThreadPid() {
    return g_main_thread_pid;
}

const char *GetProgramShortName() {
    if (NULL != g_program_short_name) {
        return g_program_short_name;
    }
    return "UNKNOWN";
}

pid_t GetTID() {
	return getpid();
}

