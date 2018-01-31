#include "log/log.h"

namespace yamq {
    bool InitLog() {
        DPRINT("Init log\n");
        return true;
    }

    bool ShutdownLog() {
        DPRINT("Shutdown log\n");
        return true;
    }
} // yamq
