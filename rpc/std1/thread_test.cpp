#include "thread.h"

void threadFunc() {
    printf("child thread id = %d \n",std2::this_thread::GetTid());
}

int main() {
    printf("main thread id = %d \n",std2::this_thread::GetTid());
    std2::Thread thread(threadFunc);
    thread.Join();
}
