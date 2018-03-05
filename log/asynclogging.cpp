#include "log/asynclogging.h"
#include "log/logfile.h"

namespace yamq {

std::unique_ptr<log::LogWorker> g_logworkerptr;
bool g_asyncLoggingStarted = false;
bool initAsyncLogging() {
   LOG(WARNING) << "init async logging";
   g_logworkerptr.reset(new log::LogWorker(2,300,getProjectDirname(),getProjectName())); 
   LOG(WARNING) << "init async logging1";
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

namespace log {
void LogWorker::threadFunc() {
    fprintf(stderr,"hello world\n");
    std::vector<BufferPtr> buffersWriteable;
    std::vector<BufferPtr> buffersWriten;
    BufferPtr buffer(new Buffer());
    LogFile output(_dirname.c_str(),_filename.c_str());
    
    for(;;) {
        {
        std::unique_lock<std::mutex> lock(_mutex);
        // 消费者等待
        fprintf(stderr,"hello world1\n");
        _condition.wait_for(lock,std::chrono::milliseconds(_flushInterval),[this](){return _stop || !_buffersFilled.empty(); });
        fprintf(stderr,"hello world2\n");
        // 0&1库
        if (_buffersFilled.empty()) {
            std::swap(_buffer,buffer);
        }
        }
        if (_buffersFilled.empty()) {
            output.append(buffer->data(),buffer->length()); 
        } else {
            std::unique_lock<std::mutex> lock(_mutex);
            BufferPtr buf;
            while (!_buffersFilled.empty()) {
                buf.reset(_buffersFilled.front().release());
                _buffersFilled.pop();

                // 消费
                lock.unlock();
                output.append(buf->data(),buf->length());
                lock.lock();

                if (_buffersAvailiable.empty()) {
                    LOG(WARNING) << "Put availiable buffer in backend!!";
                }
                _buffersAvailiable.push_back(std::move(buffer));
            }
        }
        output.flush();
        if (_stop) break;
    }
}

void LogWorker::flush() {
    std::lock_guard<std::mutex> guard(_mutex);
    _condition.notify_one();
}

void LogWorker::append_async(const char *data,size_t len) {
    fprintf(stderr,"hello world3\n");
    std::lock_guard<std::mutex> guard(_mutex);

    if (_stop) return;
    fprintf(stderr,"hello world4\n");

    if (_buffer.get() && _buffer->remain() < len) {
        _buffer->append(data,len);
    } else {
        // _buffer 
        _buffersFilled.push(std::move(_buffer));
        _condition.notify_one();

        if (!_buffersAvailiable.empty()) {
            _buffer.reset(_buffersAvailiable.back().release());
            _buffersAvailiable.pop_back();
            _buffer->append(data,len);
        } else {
            LOG(WARNING) << "No availiable buffer in frontend!!";
        }
    }
}
} // log
}// yamq
