#include <iostream>
#include "log/log.h"
using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    yamq::InitLog(argv[0]);
    cout << "Hello YAMQ." << endl;
    LOG(INFO) << "Hello YAMQ.INFO";
    LOG(WARNING) << "Hello YAMQ.WARNING";
    LOG(ERROR) << "Hello YAMQ.ERROR";
    LOG(FATAL) << "Hello YAMQ.FATAL";
    yamq::ShutdownLog();
    return 0;
}
