#include <stddef.h>
#include <stdint.h>
#include <string>

namespace debug {

struct stacktrace_options {
    uint16_t skip_start_frames;
    uint16_t skip_end_frames;
    uint16_t max_frames;
};

size_t stacktrace_write(char *buf, size_t bufsz, const stacktrace_options *options = NULL);

/**
 * stacktrace_write 包装
 */
std::string stacktrace(const stacktrace_options *options = NULL);
}
