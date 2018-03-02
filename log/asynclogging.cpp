#include "log/asynclogging.h"
#include "log/logfile.h"

namespace yamq {
namespace log {
void LogWorker::threadFunc() {
    std::vector<BufferPtr> buffersWriteable;
    std::vector<BufferPtr> buffersWriten;
    BufferPtr buffer(new Buffer());
    LogFile output(_dirname.c_str(),_filename.c_str());
    
    for(;;) {
        {
        std::unique_lock<std::mutex> lock(_mutex);
        // 消费者等待
        _condition.wait_for(lock,std::chrono::milliseconds(_flushInterval),[this](){return _stop || !_buffersFilled.empty(); });

        assert(!_buffersFilled.empty());

        _buffersFilled.push_back(std::move(_currentBuffer));
        if (_buffersAvailiable.empty()) {
            _currentBuffer.reset(_buffersAvailiable.back().release());
            _buffersAvailiable.pop_back();
        } else {
            _currentBuffer.reset(nullptr);
            LOG(WARNING) << "No availiable buffer in backend!";
        }
        buffersWriteable = std::move(_buffersFilled);
        }

        for ( auto ptr = buffersWriteable.begin();ptr != buffersWriteable.end() ; ++ptr) {
            output.append((*ptr)->data(),(*ptr)->length()); 
        }

        {
        std::unique_lock<std::mutex> lock(_mutex);
        std::swap(_buffersAvailiable,buffersWriten);
        }

        output.flush();

        if (_stop) break;
    }
    output.flush();
}

void LogWorker::append_async(const char *data,size_t len) {
    std::lock_guard<std::mutex> guard(_mutex);

    if (_stop) return;

    if (_currentBuffer.get() && _currentBuffer->remain() < len) {
        _currentBuffer->append(data,len);
    } else {
        // 从空瓶子中拿
        _buffersFilled.push_back(std::move(_currentBuffer));
        _condition.notify_one();

        if (!_buffersAvailiable.empty()) {
            _currentBuffer.reset(_buffersAvailiable.back().release());
            _buffersAvailiable.pop_back();
        } else {
            LOG(WARNING) << "No availiable buffer in frontend!!";
            return;
        }
        _currentBuffer->append(data,len);
    }
}
} // log
}// yamq
