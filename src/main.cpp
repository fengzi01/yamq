#include <iostream>
#include "log/log.h"
using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    yamq::InitLog();
    cout << "Hello YAMQ." << endl;
    yamq::ShutdownLog();
    return 0;
}
