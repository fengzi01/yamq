#ifndef YAMQ_ASYNC_LOGGING
#define YAMQ_ASYNC_LOGGING 
#include <stddef.h>
#include "log/logstream.h"

namespace yamq {

/**
 * 异步消息存储模块
 * 暴露四个接口：
 * 1. 启动异步日志
 * 2. 停止异步日志
 * 3. 保存日志
 * 4. 强制刷新日志
 */
bool initAsyncLogging();
bool shutdownAsyncLogging();
void asyncLoggingSave(const char *msg,size_t len);
void asyncLoggingFlush();

namespace internal {
}

namespace log {
class LogWorker {
    public:
        static const size_t kMaxLogBufferLen= 3000000; 
        /* 消息存储缓存 */
        typedef internal::FixedBuffer<kMaxLogBufferLen> BigBuffer;
    private:
};
} // log
} // yamq
#endif /* ifndef YAMQ_ASYNC_LOGGING */
