#include "stacktrace.h"
#include <string>
#include <string.h>
#define __USE_GNU
#include <stdlib.h>
#include <signal.h>
#include <execinfo.h>
#include <stdio.h>
#include <cxxabi.h>   // for __cxa_demangle

#ifndef LOG_STACKTRACE_MAX_STACKS
#define LOG_STACKTRACE_MAX_STACKS 100
#endif
#define LOG_STACKTRACE_MAX_STACKS_ARRAY_SIZE (LOG_STACKTRACE_MAX_STACKS + 1)

namespace debug {

//struct stacktrace_options {
//    uint16_t skip_start_frames;
//    uint16_t skip_end_frames;
//    uint16_t max_frames;
//};

const stacktrace_options &default_stacktrace_options() {
	static stacktrace_options opts = {0, 0, 0};
	return opts;
}

inline bool stacktrace_is_space_char(char c) { return ' ' == c || '\r' == c || '\t' == c || '\n' == c; }

struct stacktrace_symbol_group_t {
    int state;

    size_t file_start;
    size_t file_end;
    size_t func_end;
    size_t offset_end;
    size_t addr_start;
    size_t addr_end;

    const char *symbol_str;
    size_t pos;
    size_t last;
};

#define AGAIN 1
#define OK 0

// 用状态机去解析函数
int stacktrace_parse_symbol_info(stacktrace_symbol_group_t *s) {
    char ch;
    size_t pi;
    enum State {
        sw_start = 0,
        sw_file,
        sw_func,
        sw_offset,
        sw_addr,
        sw_done
    } state;

    state = static_cast<State>(s->state);
    for (pi = s->pos; pi < s->last; ++pi) {
        ch = s->symbol_str[pi];
        if (stacktrace_is_space_char(ch)) { continue; }
        switch (state) {
            case sw_start:
                s->file_start = pi;
                state = sw_file;
                break;
            case sw_file:
                if (ch == '(') {
                    s->file_end = pi;
                    state = sw_func;
                }
                break;
            case sw_func:
                if (ch == '+') {
                    s->func_end = pi;
                    state = sw_offset;
                }
                break;
            case sw_offset:
                if (ch == ')') {
                    s->offset_end = pi;
                    state = sw_addr;
                }
                break;
            case sw_addr:
                if (ch == '[') {
                    s->addr_start = pi+1;
                }
                if (ch == ']') {
                    s->addr_end = pi;
                    state = sw_done;
                    goto done;
                }
                break;
            case sw_done:
                goto done;
        }
    }
    s->pos = pi;
    s->state = state;
    return AGAIN;
done:
    return OK;
}

size_t stacktrace_write(char *buf, size_t bufsz, const stacktrace_options *options) {
	if (NULL == buf || bufsz <= 0) {
		return 0;
	}

	if (NULL == options) {
		options = &default_stacktrace_options();
	}

	size_t ret = 0;
	size_t skip_frames = static_cast<size_t>(options->skip_start_frames);
	void *stacks[LOG_STACKTRACE_MAX_STACKS_ARRAY_SIZE] = {NULL};

	size_t frames_count = backtrace(stacks, LOG_STACKTRACE_MAX_STACKS_ARRAY_SIZE);
	char **symbols = backtrace_symbols(stacks, frames_count);

    int rc = 0;

    stacktrace_symbol_group_t s;
    std::string func_name;
    std::string func_offset;
    std::string func_address;
    std::string file_name;

	if (frames_count <= skip_frames + options->skip_end_frames) {
		frames_count = 0;
	} else if (options->skip_end_frames > 0) {
		frames_count -= options->skip_end_frames;
	}

	for (size_t i = skip_frames; i < frames_count; i++) {
		int frame_id = static_cast<int>(i - skip_frames);
		if (0 != options->max_frames && frame_id >= static_cast<int>(options->max_frames)) {
			break;
		}

		if (NULL == symbols[i] || NULL == stacks[i] || 0x01 == reinterpret_cast<intptr_t>(stacks[i])) {
			break;
		}

        s.symbol_str = symbols[i];
        s.pos = 0;
        s.last = strlen(s.symbol_str);

        rc = stacktrace_parse_symbol_info(&s);
        if (OK != rc) {
            break;
        }

        const char *base = s.symbol_str;
        func_name = std::string(base+s.file_end+1,base+s.func_end);
        func_offset = std::string(base+s.func_end,base+s.offset_end);
        func_address = std::string(base+s.addr_start,base+s.addr_end);
        file_name = std::string(base+s.file_start,base+s.file_end);

        //printf("file = [%s], func_name = [%s], func_offset = [%s], func_addr = [%s]\n",file_name.c_str(),func_name.c_str(),func_offset.c_str(),func_address.c_str());

//#if defined(USING_LIBSTDCXX_ABI) || defined(USING_LIBCXX_ABI)
        if (!func_name.empty()) {
            if (func_name[0] == '_' && func_name[1] != '_') {
                // c++ function name
                int cxx_abi_status;
                char *realfunc_name = abi::__cxa_demangle(func_name.c_str(), 0, 0, &cxx_abi_status);
                if (NULL != realfunc_name) {
                    func_name = realfunc_name;
                }
                if (NULL != realfunc_name) {
                    free(realfunc_name);
                }
            } else if (func_name[0] == '_' && func_name[1] == '_'){
                // module name
            } else {
                // c function name
                func_name += "()";
            }
        }
//#endif

		int res = snprintf(buf, bufsz, "#%d: %s %s%s\r\n", frame_id, func_address.c_str(), func_name.c_str(),
		        func_offset.c_str());

		if (res <= 0) {
		    break;
		}

		ret += static_cast<size_t>(res);
		buf += res;
		bufsz -= static_cast<size_t>(res);
	}

	free(symbols);

	return ret;
}

std::string stacktrace(const stacktrace_options *options) {
    char buf[10000];
    size_t n = stacktrace_write(buf,sizeof buf,options);
    std::string ret;

    if (n > 0) {
        ret = std::string(buf,n);
    }
    return ret;
}

} // namespace debug
