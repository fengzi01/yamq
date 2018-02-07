#ifndef YAMQ_LOGFILE
#define YAMQ_LOGFILE 
#include <stddef.h>
#include <stdio.h>

class LogFile final {
    public:
        LogFile(const char *filePath,size_t rollSize);
        LogFile(const LogFile &) = delete;
        LogFile &operator=(LogFile &) = delete;
    private:
        FILE *_raw;
};
#endif /* ifndef YAMQ_LOGFILE */
