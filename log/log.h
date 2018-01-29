#include "stdio.h"
#define DPRINT(x...) printf(x)
namespace yamq {
namespace log {
    bool InitLog();
    bool ShutdownLog();
} // log
} // yamq
