#include <iostream>
#include "log/logging.h"

using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    cout << "Hello YAMQ." << endl;
    LOG(INFO) << "hello world";
    return 0;
}
