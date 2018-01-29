#include "log/log.h"

namespace yamq {
namespace log {
    bool InitLog() {
        DPRINT("Init log\n");
        return true;
    }

    bool ShutdownLog() {
        DPRINT("Shutdown log\n");
        return true;
    }
} // log
} // yamq
