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

void LogWorker::threadFunc() {
    BufferPtr buffer;
    //LogFile output(_dirname.c_str(),_filename.c_str());
    for(;;) {
        bool isTimeout = false;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            // 消费者等待
            _condition.wait_for(lock,std::chrono::milliseconds(_flushInterval),[this](){return _stop || !_buffersFilled.empty(); });
            // 0&1库
            if (_buffersFilled.empty()) {
                isTimeout = true;
                buffer.reset(_buffer.release());
            }
        }
        if (isTimeout) {
            _output->append(buffer->data(),buffer->length()); 
            buffer->reset();
            {
                // 归还buffer
                std::lock_guard<std::mutex> guard(_mutex);
                _buffersAvailiable.push_back(std::move(buffer));
            }
        } else {
            std::unique_lock<std::mutex> lock(_mutex);
            while (!_buffersFilled.empty()) {
                buffer.reset(_buffersFilled.front().release());
                _buffersFilled.pop();
                // 消费
                lock.unlock();
                _output->append(buffer->data(),buffer->length());
                // buffer还原
                buffer->reset();
                lock.lock();
//              if (_buffersAvailiable.empty()) {
//                  fprintf(stderr,"Put availiable buffer in backend!!\n");
//              }
                _buffersAvailiable.push_back(std::move(buffer));
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
        buffer.reset(_buffersFilled.front().release());
        _buffersFilled.pop();
        // 写入文件
        _output->append(buffer->data(),buffer->length());
        // 重置buf & 放回缓存池
        buffer->reset();
        _buffersAvailiable.push_back(std::move(buffer));
    }
    _output->flush();
}

void LogWorker::append_async(const char *data,size_t len) {
    std::lock_guard<std::mutex> guard(_mutex);

    if (_stop) return;

    if (_buffer.get() && _buffer->remain() > len) {
        _buffer->append(data,len);
    } else {
        // _buffer 
        if (_buffer.get()) {
            _buffersFilled.push(std::move(_buffer));
            _buffer.reset(nullptr);
            _condition.notify_one();
        }

        if (!_buffersAvailiable.empty()) {
            _buffer.reset(_buffersAvailiable.back().release());
            _buffersAvailiable.pop_back();
            _buffer->append(data,len);
        } else {
            fprintf(stderr,"No availiable buffer in frontend!!\n");
        }
    }
}
} // log
}// yamq
