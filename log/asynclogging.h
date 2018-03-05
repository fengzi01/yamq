#ifndef YAMQ_ASYNC_LOGGING
#define YAMQ_ASYNC_LOGGING 
#include <stddef.h>
#include "log/logstream.h"
#include "log/Thread.h"
#include <vector>
#include <mutex>
#include <condition_variable>
#include "log/shared_queue.h"

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
        typedef internal::FixedBuffer<kMaxLogBufferLen> Buffer;
        typedef std::unique_ptr<Buffer> BufferPtr;

        LogWorker(size_t bufSize,size_t intval,std::string dirname,std::string filename):_stop(false),_flushInterval(intval),_dirname(dirname),_filename(filename),_backend(new std1::Thread(std::bind(&LogWorker::threadFunc,this))) {
            _buffersAvailiable.reserve(bufSize);
            for ( size_t i = 0; i < bufSize; ++i )  {
                _buffersAvailiable.push_back(BufferPtr(new Buffer()));
            }

            fprintf(stderr,"construct logworker end %s %s\n",dirname.c_str(),filename.c_str());
        }

        ~LogWorker() {
            {
            std::lock_guard<std::mutex> guard(_mutex);
            _stop = true;
            _condition.notify_one();
            }
            _backend->join();
        }
        
        void append_async(const char *data,size_t len); 
        void flush();
    private:
        void threadFunc();

        /* 填满的buffer数组 */
        std::queue<BufferPtr> _buffersFilled;
        /* 可用的buffer数组 */
        std::vector<BufferPtr> _buffersAvailiable;
        BufferPtr _buffer;
        BufferPtr _bufferAvailiable;

        std::mutex _mutex;
        std::condition_variable _condition;

        bool _stop;
        const size_t _flushInterval; // milliseconds 毫秒

        std::string _dirname;
        std::string _filename;

        std::unique_ptr<std1::Thread> _backend;        
};
} // log
} // yamq
#endif /* ifndef YAMQ_ASYNC_LOGGING */
