#include <iostream>
#include "log/log.h"
using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    cout << "Hello YAMQ." << endl;
    LOG(WARNING) << "Hello YAMQ.";
    return 0;
}
