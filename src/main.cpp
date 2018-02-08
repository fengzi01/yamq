#include <iostream>
#include "log/logging.h"

using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    yamq::initLogging(argv[0]);
    LOG(TRACE) << "hello world";
    LOG(INFO) << "hello world";
    LOG(WARNING) << "hello world";
    LOG(ERROR) << "hello world";
    yamq::shutdownLogging();
    return 0;
}
