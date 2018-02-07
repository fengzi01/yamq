#ifndef YAMQ_LOGFILE
#define YAMQ_LOGFILE 
#include <stddef.h>
#include <stdio.h>
#include <string>
#include <memory> // unique_ptr
#include <algorithm> // std::move
#include "unistd.h"
#include <string.h>

namespace yamq {
namespace internal {
/* 封装文件指针 */
class FileWrapper {
    public:
        FileWrapper(const char* filepath,const char *mode):_filepath(filepath),_filemode(mode) {
            if ( NULL == (_raw = ::fopen(filepath,mode))) {
                perror("Could't create file");
                _valid = false;
            } else {
                _valid = true;
            }
        }

        FileWrapper(const FileWrapper&) = delete;
        FileWrapper &operator=(FileWrapper&) = delete;
        ~FileWrapper() {
            ::fclose(_raw);
        }

        size_t write(const void *buf,size_t len) {
            if (!valid() || len <= 0) {return 0;}
            size_t remain = len;
            do {
                size_t x = ::fwrite(buf,len,1,_raw);
                if (0 == x) {
                    int err = ferror(_raw);
                    if (err) {
                        perror("Could't write file");
                    }
                    break;
                }
                remain -= x;
            }while(0 == remain);
            return len - remain;
        }

        // FIXME
        size_t read(const void *buf) {(void)buf; return 0;}

        // FIXME
        int seek(long offset,int base) {
            if (!valid()) {return -1;}
            (void)offset,(void)base;
            return 0;
        }

        void flush() {
            fflush(_raw);
        }

        bool valid() {return _valid;}
    private:
        std::string _filepath;
        std::string _filemode;
        bool _valid;
        FILE *_raw;
};
}// internal

namespace log {
class LogFile {
    public:
        LogFile(const char *basename,const char *filename,uint32_t rollsize);
        LogFile(const LogFile &) = delete;
        LogFile &operator=(LogFile &) = delete;

        /* 追加日志 */
        size_t append(const char *data,size_t len);
        void flush() {_file->flush();}

        /* 整点滚动日志文件 */
        /* 日志滚动 */
        bool roll();
    private:
        std::unique_ptr<internal::FileWrapper> _file;
        std::string _basename;
        std::string _filename;
        uint32_t _rollsize;
        std::string _extension; // 扩展名
        /* 写入字符数 */
        uint64_t _writenChars;
        int _lastHour;

        constexpr static const char *FILE_MODE = "ae";
        static const size_t kMaxRollSize = 2*1000*1000*1000;
};
} // log
namespace detail {

} // detail
} // yamq
#endif /* ifndef YAMQ_LOGFILE */
