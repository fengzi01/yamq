#include <iostream>
#include "log/log.h"
using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    yamq::InitLog(argv[0]);
    cout << "Hello YAMQ." << endl;
    LOG(WARNING) << "Hello YAMQ.";
    LOG(WARNING) << "Hello YAMQ.";
    LOG(WARNING) << "Hello YAMQ.";
    LOG(WARNING) << "Hello YAMQ.";
    yamq::ShutdownLog();
    return 0;
}
