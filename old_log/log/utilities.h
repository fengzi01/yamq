#ifndef _LOG_UTILITIES
#define _LOG_UTILITIES 
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

typedef long long int64;
typedef double WallTime;

int64 CycleClock_Now();
WallTime WallTime_Now() ;
const char* const_basename(const char* filepath); 

void InitLogUtilities(const char *argv0);
void ShutdownLogUtilities();
int GetMainThreadPid(); 
const char *GetProgramShortName();

pid_t GetTID(); 
#endif /* ifndef _LOG_UTILITIES */
