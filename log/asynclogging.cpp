#include "log/asynclogging.h"
namespace yamq {

std::unique_ptr<log::LogWorker> g_logworkerptr;
bool g_asyncLoggingStarted = false;
bool initAsyncLogging() {
   g_logworkerptr.reset(new log::LogWorker(3,300,getProjectDirname(),getProjectName())); 
   g_asyncLoggingStarted = true;

   g_loggingSaveFunc = asyncLoggingSave;
   g_loggingFlushFunc = asyncLoggingFlush;

   return true;
}

void asyncLoggingSave(const char *msg,size_t len) {
    if (g_asyncLoggingStarted) {
        g_logworkerptr->append_async(msg,len);
    }
}

void asyncLoggingFlush() {
    if (g_asyncLoggingStarted) {
        g_logworkerptr->flush();
    }
}

bool shutdownAsyncLogging() {
    g_logworkerptr.reset(nullptr);
    return true;
}

namespace log {
LogWorker::LogWorker(size_t bufSize,size_t intval,std::string dirname,std::string filename):
    _buffer(new Buffer()),
    _stop(false),
    _flushInterval(intval),
    _output(new LogFile(dirname.c_str(),filename.c_str())), 
    _backend(new std1::Thread(std::bind(&LogWorker::threadFunc,this))) {
        _buffersAvailiable.reserve(bufSize);
        for ( size_t i = 0; i < bufSize; ++i )  {
            _buffersAvailiable.push_back(BufferPtr(new Buffer()));
        }
}

LogWorker::~LogWorker() {
    {
        std::lock_guard<std::mutex> guard(_mutex);
        _stop = true;
        _condition.notify_one();
    }
    _backend->join();
}

/* 消费者线程 */
void LogWorker::threadFunc() {
    BufferPtr buffer;
    for(;;) {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            // 消费者等待
            _condition.wait_for(lock,std::chrono::milliseconds(_flushInterval),[this](){return _stop || !_buffersFilled.empty(); });
            if (_stop && _buffer.get()) {
                // 消费所有内容
                _buffersFilled.push(std::move(_buffer));
                _buffer.reset(nullptr);
            }
            if (_buffersFilled.empty()) {
                if (!_buffer.get() || _buffer->length() == 0) {
                    continue;
                }
                buffer = std::move(_buffer);

                lock.unlock();
                _output->append(buffer->data(),buffer->length()); 
                buffer->reset();
                lock.lock();

                _buffersAvailiable.push_back(std::move(buffer));
            } else {
                while (!_buffersFilled.empty()) {
                    buffer = std::move(_buffersFilled.front());
                    _buffersFilled.pop();

                    lock.unlock();
                    _output->append(buffer->data(),buffer->length());
                    buffer->reset();
                    lock.lock();

                    _buffersAvailiable.push_back(std::move(buffer));
                }

            }
        }
        _output->flush();
        if (_stop) {
            break;
        }
    }
}

void LogWorker::flush() {
    std::lock_guard<std::mutex> guard(_mutex);
    BufferPtr buffer;
    if (_buffer.get()) {
        _buffersFilled.push(std::move(_buffer));
        _buffer.reset(nullptr);
    }
    while (!_buffersFilled.empty()) {
        buffer = std::move(_buffersFilled.front());
        _buffersFilled.pop();
        // 写入文件
        _output->append(buffer->data(),buffer->length());
        // 重置buf & 放回缓存池
        buffer->reset();
        _buffersAvailiable.push_back(std::move(buffer));
    }
    _output->flush();
}

/* 生产者线程 */
void LogWorker::append_async(const char *data,size_t len) {
    std::lock_guard<std::mutex> guard(_mutex);

    if (_stop) return;

    if (_buffer.get() && _buffer->remain() > len) {
        _buffer->append(data,len);
    } else {
        if (_buffer.get()) {
            _buffersFilled.push(std::move(_buffer));
            _buffer.reset(nullptr);
            // 通知消费者线程
            _condition.notify_one();
        }

        // 从内存池中获取可用buffer
        if (!_buffersAvailiable.empty()) {
            _buffer = std::move(_buffersAvailiable.back());
            _buffersAvailiable.pop_back();

            _buffer->append(data,len);
        } else {
            // 获取失败
            fprintf(stderr,"No availiable buffer in frontend!!\n");
        }
    }
}
} // log
}// yamq
