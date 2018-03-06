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
class FileWrapper {
    public:
        FileWrapper(const char* filepath,const char *mode):_filepath(filepath),_filemode(mode) {
            open();
        }

        FileWrapper(const FileWrapper&) = delete;
        FileWrapper &operator=(FileWrapper&) = delete;
        ~FileWrapper() {
            ::fclose(_raw);
        }

        size_t write(const void *buf,size_t len) {
            if (!valid() || len <= 0) {return 0;}
            if (checkIfOpen() != 0) {
                return 0;
            }
            size_t remain = len;
            size_t x = 0;
            do {
                x = ::fwrite(buf,sizeof(char),len,_raw);
                if (0 == x) {
                    int err = ferror(_raw);
                    if (err) {
                        perror("Could't write file");
                    }
                    break;
                }
                remain -= x;
            } while(0 != remain);
//            LOG(TRACE) << "remain:" << remain;
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
        int open() {
            if ( NULL == (_raw = ::fopen(_filepath.c_str(),_filemode.c_str()))) {
                perror("Could't create file");
                _valid = false;
            } else {
                _valid = true;
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

        static const size_t bufSize = 1000;
        char _buf[bufSize];
};
}// internal

namespace log {
void rollLog();

class LogFile {
    public:
        LogFile(const char *basename,const char *filename,uint32_t rollSize = kMaxRollSize,const char *extension = "log");
        LogFile(const LogFile &) = delete;
        LogFile &operator=(LogFile &) = delete;

        /* 追加日志 */
        size_t append(const char *data,size_t len);
        void flush() {_file->flush();}

    private:
        /* 日志滚动 */
        bool roll();

        std::unique_ptr<internal::FileWrapper> _file;
        std::string _basename;
        std::string _filename;
        std::string _extension; // 扩展名

        /* 上一次roll的时间点 */
        Timestamp _lastTimestamp;
        /* 上一次roll的文件路径 */
        std::string _lastFilepath;
        uint32_t _rollIndex;

        /* 自上一次roll写入字符数 */
        uint64_t _writenChars;
        /*最大日志大小*/
        uint64_t _kRollSize;

        std::mutex _mutex;

        constexpr static const char *FILE_MODE = "ae";
        static const uint64_t kMaxRollSize = 2*1000*1000*1000;
};
} // log
namespace detail {

} // detail
} // yamq
#endif /* ifndef YAMQ_LOGFILE */
