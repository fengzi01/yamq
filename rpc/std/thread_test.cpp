#include "thread.h"
#include <vector>

void threadFunc() {
    printf("child thread id = %d \n",std2::this_thread::GetTid());
}

int main() {
    printf("main thread id = %d \n",std2::this_thread::GetTid());
    std::vector<std2::Thread *> v;
    for (int i = 0; i < 100; i++) {
        v.push_back(new std2::Thread(threadFunc));
    }
    for (int i = 0; i < 100; i++) {
        v[i]->Join();
        delete v[i];
    }
}
