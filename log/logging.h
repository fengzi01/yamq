#ifndef _YAMQ_LOGGING_H
#define _YAMQ_LOGGING_H

#include "stdio.h"
#include "stddef.h" // size_t

#include "log/logstream.h"
#include "memory"

#define ASYNC_LOGGING 1

#define DPRINT(format,...) printf(format,##__VA_ARGS__)
namespace yamq {

/**
 * 日志等级
 */
const int LOG_TRACE = 0,LOG_INFO = 1, LOG_WARNING = 2, LOG_ERROR = 3, LOG_FATAL = 4;
const size_t  NUM_LOG_LEVELS = 5;
const char*const LogLevelNames[NUM_LOG_LEVELS] = {
    "TRACE","INFO", "WARNING", "ERROR", "FATAL"
};

/*
 * 日志解析模块，日志存储模块交互接口
 */
typedef void (*LoggingSaveFunc)(const char *message,size_t len);
typedef void (*LoggingFlushFunc)();

extern LoggingSaveFunc g_loggingSaveFunc;
extern LoggingFlushFunc g_loggingFlushFunc;

#ifdef SYNC_LOGGING
namespace log {
class LogFile;
}
extern std::unique_ptr<log::LogFile> g_logfileptr;
extern bool g_syncLoggingStarted;

void syncLoggingSave(const char *,size_t); 
void syncLoggingFlush(); 
#endif 

bool initLogging(const char *);
bool shutdownLogging();

namespace log {


/*
const int& logLevel() {
    return LOG_INFO;
}
*/

/* 日志快照 */
class LogCapture final {
    public:
        ~LogCapture();
        LogStream &stream() {return _stream;}
        LogCapture(const char *file, int line, const char *function, int level);

    private:
        /* 添加日志前缀 */
        void add_prefix();
        const char *_file;
        const int _line;
        const char *_function;
        const int _level;
        LogStream _stream;
};

} // yamq::log

/**
 * 占位符
 */
class NullStream {
    public:
        NullStream() = default;
        ~NullStream() = default;
        template<class T> inline NullStream &operator<<(T t){(void)t;return *this;}
        NullStream &stream() { return *this; }
};
} // yamq


#define LOG(level) _COMPACT_LOG_ ## level.stream()

#define YAMQ_STRIP_LOG 0
#if YAMQ_STRIP_LOG <= 0
#define _COMPACT_LOG_TRACE yamq::log::LogCapture( \
    __FILE__, __LINE__,static_cast<const char*>(__PRETTY_FUNCTION__), yamq::LOG_TRACE)
#else
#define _COMPACT_LOG_INFO yamq::NullStream()
#endif

#if YAMQ_STRIP_LOG <= 1
#define _COMPACT_LOG_INFO yamq::log::LogCapture( \
    __FILE__, __LINE__,static_cast<const char*>(__PRETTY_FUNCTION__), yamq::LOG_INFO)
#else
#define _COMPACT_LOG_INFO yamq::NullStream()
#endif
#if YAMQ_STRIP_LOG <= 2
#define _COMPACT_LOG_WARNING yamq::log::LogCapture( \
    __FILE__, __LINE__,static_cast<const char*>(__PRETTY_FUNCTION__), yamq::LOG_WARNING)
#else
#define _COMPACT_LOG_WARNING yamq::NullStream()
#endif
#if YAMQ_STRIP_LOG <= 3
#define _COMPACT_LOG_ERROR yamq::log::LogCapture( \
    __FILE__, __LINE__,static_cast<const char*>(__PRETTY_FUNCTION__), yamq::LOG_ERROR)
#else
#define _COMPACT_LOG_ERROR yamq::NullStream()
#endif
#if YAMQ_STRIP_LOG <= 4
#define _COMPACT_LOG_FATAL yamq::log::LogCapture( \
    __FILE__, __LINE__,static_cast<const char*>(__PRETTY_FUNCTION__), yamq::LOG_FATAL)
#else
#define _COMPACT_LOG_FATAL yamq::NullStream()
#endif

#endif /* ifndef _YAMQ_LOGGING_H */
