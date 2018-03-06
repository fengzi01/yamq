#ifndef YAMQ_LOGFILE
#define YAMQ_LOGFILE 
#include <stddef.h>
#include <stdio.h>
#include <string>
#include <memory> // unique_ptr
#include <algorithm> // std::move
#include <unistd.h>
#include <string.h>
#include "log/utilities.h"
#include "log/logging.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <mutex>

namespace yamq {
namespace internal {
/* 封装文件指针 */
class AppendFileWrapper final {
    public:
        AppendFileWrapper(const char* filepath,const char *mode):
            _filepath(filepath),
            _filemode(mode),_writenChars(0) {
            open();
        }

        AppendFileWrapper(const AppendFileWrapper&) = delete;
        AppendFileWrapper &operator=(AppendFileWrapper&) = delete;
        ~AppendFileWrapper() {
            ::fclose(_raw);
        }

        size_t append(const void *buf,size_t len) {
            if (!valid() || len <= 0) {return 0;}
            if (checkIfOpen() != 0) {
                return 0;
            }
            size_t remain = len;
            size_t x = 0;
            size_t writen = 0;
            
            do {
#ifdef fwrite_unlocked
                x = ::fwrite_unlocked(buf,sizeof(char),len,_raw);
#else
                x = ::fwrite(buf,sizeof(char),len,_raw);
#endif
                if (0 == x) {
                    int err = ferror(_raw);
                    if (err) {
                        perror("Could't write file");
                    }
                    break;
                }
                remain -= x;
            } while(0 != remain);
            writen = len - remain;
            _writenChars += writen;
            return writen;
        }

        uint64_t writenChars() {return _writenChars;}

        void flush() {
            fflush(_raw);
        }

        bool valid() {return _valid;}
    private:
        int open() {
            if ( NULL == (_raw = ::fopen(_filepath.c_str(),_filemode.c_str()))) {
                perror("Could't create file");
                _valid = false;
            } else {
                _valid = true;
                _writenChars = 0;
                setbuffer(_raw,_buf,bufSize);

                // 存储文件属性
                int fd = fileno(_raw); 
                return fstat(fd,&_st);
            }
            return -1;
        }
        /* 检测文件描述符并重新打开文件 */
        int checkIfOpen() {
           struct stat st;
           int err = 0;
           if ( (err=::stat(_filepath.c_str(),&st)) != 0) {
               fprintf(stderr,"file is not exist!\n");
               return open();
           } else {
               if (_st.st_ino != st.st_ino) {
                   // 文件不一致
                   fprintf(stderr,"file st_ino don't equal!\n");
                   ::fclose(_raw);
                   return open();
               } else {
                   // dummy
                   return 0;
               }
           }
           return -1;
        }

        std::string _filepath;
        std::string _filemode;
        bool _valid;
        FILE *_raw;
        struct stat _st;
        uint64_t _writenChars;

        static const size_t bufSize = 1000;
        char _buf[bufSize];
};
}// internal

namespace log {

using internal::AppendFileWrapper;
class LogFile {
    public:
        LogFile(const char *basename,const char *filename,uint64_t rollSize = kMaxRollSize,const char *extension = "log");
        LogFile(const LogFile &) = delete;
        LogFile &operator=(LogFile &) = delete;

        /* 追加日志 */
        size_t append(const char *data,size_t len);
        void flush() {_file->flush();}

    private:
        /* 日志滚动 */
        bool roll();

        std::unique_ptr<AppendFileWrapper> _file;
        std::string _basename;
        std::string _filename;
        std::string _extension; // 扩展名

        /* 上一次roll的时间点 */
        Timestamp _lastTimestamp;
        /* 上一次roll的文件路径 */
        std::string _lastFilepath;
        uint32_t _rollIndex;

        /*最大日志大小*/
        uint64_t _kRollSize;

        std::mutex _mutex;

        static constexpr const char *FILE_MODE = "ae";
        static const uint64_t kMaxRollSize = 2LLU*1024*1024*1024; // 2G字节
};
} // log
namespace detail {

} // detail
} // yamq
#endif /* ifndef YAMQ_LOGFILE */
