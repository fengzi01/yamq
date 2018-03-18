#pragma once
#include <functional>
#include <unistd.h>  // pid_t
#include <pthread.h>

namespace std2 {
class Thread {
    public:
       typedef std::function<void ()> ThreadFunc;
       struct ThreadData;

       explicit Thread(const ThreadFunc&);
       explicit Thread(ThreadFunc &&);

       ~Thread();

       Thread(Thread &) = delete;
       Thread(const Thread &) = delete;
       Thread& operator=(const Thread&) = delete;

       void Join();
       void Detach();

       pid_t GetTid() { return _tid; } 

       bool Joinable() const noexcept {
           return !_joined;
       }


    private:
       void startThread();
       pthread_t _ptid;
       ThreadFunc _func;
       bool _joined;
       pid_t _tid;
};

namespace this_thread {
    extern __thread int tid;
    extern __thread char tid_str[32];
    extern __thread int tid_strlen;
    //extern __thread const char* tname;

    static_assert(std::is_same<int,pid_t>::value,"pid_t must is same as int!");
    int GetTid();
    const char *GetTid_str() { return tid_str; }
    int GetTid_strlen() { return tid_strlen; }
}
}
