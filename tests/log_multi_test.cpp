#include "log/logfile.h"
#include "log/logging.h"

#include "base/std/thread.h"

static void mysleep(int ms)
{
    timespec t = { 0, 10*ms };
    nanosleep(&t, NULL);
}

void threadFunc() {
    int tid = std2::this_thread::GetTid();
    while (true) {
        LOG(TRACE) << "hello world tid = [" << tid << "]";
        LOG(INFO) << "hello world tid = [" << tid << "]";
        LOG(WARNING) << "hello world tid = [" << tid << "]";
        LOG(ERROR) << "hello world tid = [" << tid << "]";
    //    LOG(FATAL) << "hello world";
        mysleep(10);
    }
}

int main(int argc, char *argv[])
{
    int n = 5;
    std::vector<std::unique_ptr<std2::Thread>> vec;
    for (int i = 0; i < n; ++i) {
        vec[i].reset(new std2::Thread(threadFunc));
    }
    for (int i = 0; i < n; ++i) {
        vec[i]->Join();
    }
    return 0;
}
