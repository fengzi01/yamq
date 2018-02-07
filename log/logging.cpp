#include "log/logging.h"
#include "utilities.h"

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

bool initLogging() {
    return true;
}

bool shutdownLogging() {
    return true;
}

namespace log {
namespace {
    size_t logtime(char *buf,yamq::Timestamp now) {
        size_t len = 22;
        ReadableTime tm;
        time_t time = static_cast<time_t>(now / 1000000);
        uint32_t microSeconds = now % 1000000;
        ::gmtime_r(&time,&tm);
        // 170703 22:04:05.242153
        snprintf(buf,22,"%02d%02d%02d %02d:%02d:%02d.%06d",tm.tm_year,tm.tm_mon,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec,microSeconds);
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
        size_t len = logtime(buf.current(),yamq::nowTime());
        buf.offset(len);
    }
    _stream << " " << static_cast<int>(getTid()) << " ";
    _stream << _file << ":" << _line << "] "; 
}

LogCapture::~LogCapture() {
    LogStream::Buffer &buf = stream().buffer();
    buf.append("\n",2);
    // 析构函数中写入日志存储模块
    g_loggingSaveFunc(buf.data(),buf.length());
}

}//yamq::log
} // yamq
