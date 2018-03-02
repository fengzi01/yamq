#include "ThreadPool.h"

namespace std1 {
void ThreadPool::threadFunc() {
    for(;;) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _readyCondition.wait(lock,[this]{return this->_stop || !this->_tasks.empty();});
            if (_stop && _tasks.empty()) {
                return;
            }
            // 如果tasks任务没执行完成，需要继续执行
            task = std::move(_tasks.front());
            _tasks.pop();

            if (_maxTaskSize > 0) {
                // notify enqueue thread
                _fullCondition.notify_one();
            }
        }
        // 执行
        task();
    }
}

ThreadPool::ThreadPool(size_t kThreadNum,size_t taskSize):_maxTaskSize(taskSize),_stop(false) {
    for (size_t i = 0;i < kThreadNum; ++i) {
        // FIXME
        std::unique_ptr<Thread> ptr(new Thread(std::bind(&ThreadPool::threadFunc,this)));
        _workers.push_back(std::move(ptr));
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> guard(_mutex);
        _stop = true;
    }
    _readyCondition.notify_all();
    for (auto ptr =  _workers.begin();ptr != _workers.end();++ptr) {
        (*ptr)->join();
    }
}

void ThreadPool::addTask(Task &&task) {
    std::unique_lock<std::mutex> lock(this->_mutex);
    //fprintf(stderr,"add task\n");
    if (_maxTaskSize > 0 && _tasks.size() >= _maxTaskSize) {
        this->_fullCondition.wait(lock,
             [this] {
                return this->_stop || this->_tasks.size() < this->_maxTaskSize;
             }
        );
    }

    if (_stop) {
        return;
    }

    _tasks.push(std::move(task));
    _readyCondition.notify_one();
    //fprintf(stderr,"add task end\n");
}

} // std1
