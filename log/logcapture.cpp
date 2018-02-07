#include "log/logcapture.h"
using namespace yamq::log;
void defaultOutputFunc(const char* msg,size_t len) {
    printf("message:%s",msg);
    (void)len;
}
void defaultFlushFunc(){}

MsgOutputFunc g_outputFunc = defaultOutputFunc;
MsgFlushFunc g_flushFunc  = defaultFlushFunc;

LogCapture::LogCapture(
        const char *file, 
        const int line, 
        const char *function, 
        int level
        ):_file(file),_line(line),_function(function),_level(level){
}

LogCapture::~LogCapture() {
    LogStream::Buffer &buf = stream().buffer();
    // 析构函数中写入日志存储模块
    yamq::log::g_outputFunc(buf.data(),buf.length());
}

