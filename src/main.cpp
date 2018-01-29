#include <iostream>
#include "log/log.h"
using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    yamq::log::InitLog();
    cout << "Hello YAMQ." << endl;
    yamq::log::ShutdownLog();
    return 0;
}
