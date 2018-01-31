#include "stdio.h"
#define DPRINT(x...) printf(x)
namespace yamq {
    bool InitLog();
    bool ShutdownLog();
} // yamq
