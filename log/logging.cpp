#include "log/logging.h"

namespace yamq {
namespace log {

    void defaultOutputFunc(const char* msg,size_t len) {
        printf("message:%s",msg);
        (void)len;
    }
    void defaultFlushFunc(){}

    MsgOutputFunc g_outputFunc = defaultOutputFunc;
    MsgFlushFunc g_flushFunc  = defaultFlushFunc;

}
    bool initLogging() {
        return true;
    }

    bool shutdownLogging() {
        return true;
    }
} // yamq
