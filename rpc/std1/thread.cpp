#include "rpc/std1/thread.h"
#include <utility> // std::forword
#include <memory>
#include <stdlib.h>
#include <sys/syscall.h>

namespace std2 {

/* thread data 封装 */
struct Thread::ThreadData {
    ThreadFunc _func;
    pid_t *_tid;
    ThreadData (const ThreadFunc& func,pid_t *tid)
        :_func(func),_tid(tid) {}
    void runInThread() {
        *_tid = this_thread::GetTid(); // 初始化tid
        try {
            _func();
        }
        catch (const std::exception& ex)
        {
            fprintf(stderr, "exception caught in Thread %d\n", this_thread::GetTid());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        }
        catch (...)
        {
            fprintf(stderr, "unknown exception caught in Thread %d\n", this_thread::GetTid());
            throw; // rethrow
        }
    }
};

/* 使得C++编译器不添加 xxstd1xxnative_thread_routine修饰符 */
/* 所以这里不加也没有问题 */
extern "C" {
static void* native_thread_routine(void* p) {
    std::unique_ptr<Thread::ThreadData> data(static_cast<Thread::ThreadData*>(p));
    data->runInThread();
    return nullptr;
}
}

Thread::Thread(const ThreadFunc &func):_func(func),_joined(false),_tid(0) {
    startThread();
}

Thread::Thread(ThreadFunc &&func):_func(func),_joined(false),_tid(0) {
    startThread();
}

void Thread::startThread() {
    std::unique_ptr<ThreadData> data(new ThreadData(_func,&_tid));
    if (pthread_create(&_ptid,NULL,&native_thread_routine,data.get())) {
        perror("pthread_create call fail!");
    }
    // 销毁前释放原始指针控制权
    data.release();
}

void Thread::Join() {
    if (!_joined) {
        pthread_join(_ptid,NULL);
        _joined = true;
    }
}

void Thread::Detach() {
    if (_joined) {
        fprintf(stderr,"detach already joined thread!");
        return;
    }
    pthread_detach(_ptid);
}
namespace this_thread {
    __thread int tid = 0;
    __thread char tid_str[32];
    __thread int tid_strlen = 0;
    //__thread const char* tname;
    
    static void cacheTid() {
        if (tid == 0) {
            tid = static_cast<pid_t>(::syscall(SYS_gettid));
            tid_strlen = snprintf(tid_str, sizeof tid_str, "%5d ", tid);
        }
    }

    int GetTid() {
        if (__builtin_expect(tid == 0, 0))
        {
            cacheTid();
        }
        return tid;
    }
}

namespace {
void afterFork()
{
    std2::this_thread::tid = 0;
    std2::this_thread::GetTid();
}
class ThreadLocalInitializer
{
    public:
        ThreadLocalInitializer()
        {
            std2::this_thread::tid = 0;
            std2::this_thread::GetTid();
            pthread_atfork(NULL, NULL, &afterFork);
        }
};
ThreadLocalInitializer none;//全局变量，初始化就为主线程生成线程信息 
}
}
