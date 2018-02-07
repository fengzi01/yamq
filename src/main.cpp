#include <iostream>
#include "log/logging.h"

using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    cout << "Hello YAMQ." << endl;
    LOG(TRACE) << "hello world";
    LOG(INFO) << "hello world";
    LOG(WARNING) << "hello world";
    LOG(ERROR) << "hello world";
    return 0;
}
