#ifndef _STD1_THREADPOOL_H
#define _STD1_THREADPOOL_H 
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "Thread.h"

namespace std1 {
class ThreadPool {
    public:
        typedef std::function<void()> Task;
        ThreadPool(size_t kThreadNum,size_t taskSize = 0);
        ~ThreadPool();

        ThreadPool(ThreadPool &) = delete;
        ThreadPool(const ThreadPool &) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        void addTask(Task &&task);
        size_t getTaskSize() {
            std::lock_guard<std::mutex> guard(_mutex);
            return _tasks.size();
        }
    private:
        void threadFunc();
        std::mutex _mutex;
        std::condition_variable _readyCondition;
        std::condition_variable _fullCondition;

        std::vector< std::unique_ptr<std1::Thread> > _workers;
        std::queue< Task > _tasks;
        bool _stop;

        size_t _maxTaskSize;
};
} // std1
#endif /* ifndef _STD1_THREADPOOL_H */
