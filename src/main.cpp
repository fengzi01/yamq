#include <iostream>
#include "log/logging.h"

static void mysleep(int ms)
{
    timespec t = { 0, 10*ms };
    nanosleep(&t, NULL);
}

int main(int argc, char *argv[])
{
    yamq::initLogging(argv[0]);
    while (true) {
        LOG(TRACE) << "hello world";
        LOG(INFO) << "hello world";
        LOG(WARNING) << "hello world";
        LOG(ERROR) << "hello world";
        mysleep(10);
    }
    yamq::shutdownLogging();
    return 0;
}
