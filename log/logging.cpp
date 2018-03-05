#include "log/logging.h"
#include "utilities.h"
#include "log/logfile.h"
#include "log/asynclogging.h"

namespace yamq {

void defaultLoggingSave(const char* msg,size_t len) {
    /* 打印到标准错误输出 */
    fwrite(msg,len,1,stderr);
}
void defaultLoggingFlush() {
    fflush(stderr);
}

LoggingSaveFunc g_loggingSaveFunc = defaultLoggingSave;
LoggingFlushFunc g_loggingFlushFunc = defaultLoggingFlush;

#ifdef SYNC_LOGGING
bool g_syncLoggingStarted = false;
std::unique_ptr<log::LogFile> g_logfileptr;
void syncLoggingSave(const char *msg,size_t len) {
    if (g_syncLoggingStarted) {
        g_logfileptr->append(msg,len);
    }
}
void syncLoggingFlush() {
    if (g_syncLoggingStarted) {
        g_logfileptr->flush();
    }
}
#endif

bool initLogging(const char *argv0) {
    initUtilities(argv0);
#ifdef SYNC_LOGGING
    using yamq::log::LogFile;
    // 初始化
    g_logfileptr.reset(new LogFile(getProjectDirname(),getProjectName()));
    g_syncLoggingStarted = true;

    g_loggingSaveFunc = syncLoggingSave;
    g_loggingFlushFunc = syncLoggingFlush;

    LOG(WARNING) << "init log with sync mode";
#endif
#ifdef ASYNC_LOGGING
    initAsyncLogging();
    LOG(WARNING) << "init log with async mode";
#endif
    return true;
}


bool shutdownLogging() {
    return true;
}

namespace log {
namespace {
    size_t logtime(char *buf) {
        size_t len = 22;
        ReadableTime tm;
        Timestamp now = nowTime(&tm);
        uint32_t microSeconds = now % 1000000;
        // 170703 22:04:05.242153
        snprintf(buf,len,"%02d%02d%02d %02d:%02d:%02d.%06d",tm.tm_year,tm.tm_mon,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec,microSeconds);
        return len;
    }
}

void LogCapture::addprefix() {
    /*
     * Log line format: [IWEF]mmdd hh:mm:ss.uuuuuu threadid file:line] msg
     * I170703 22:04:05.242153  6569 glog_test2.cpp:7] Hello,GLOG!
     */
    LogStream::Buffer &buf = _stream.buffer();
    buf.append(LogLevelNames[_level],1); // FIXME _level
    
    if (buf.remain() >= 22) {
        size_t len = logtime(buf.current());
        // FIXME ? offset not equal 22 ?
        buf.offset(len-1);
    }
    _stream << " " << static_cast<int>(getTid()) << " ";
    _stream << _file << ":" << _line << "] "; 
}

LogCapture::~LogCapture() {
    LogStream::Buffer &buf = stream().buffer();
    buf.append("\n",1);
    // 析构函数中写入日志存储模块
    g_loggingSaveFunc(buf.data(),buf.length());
}

}//yamq::log
} // yamq
