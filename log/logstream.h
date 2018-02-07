#ifndef _YAMQ_LOG_LOGSTREAM
#define _YAMQ_LOG_LOGSTREAM 

#include <stddef.h> // size_t
#include <string.h>
#include <string>

namespace yamq {
namespace internal {

template<size_t bufsize>
class FixedBuffer final {
    public:
        FixedBuffer():_cur(_data){};
        FixedBuffer(const FixedBuffer &) = delete;
        FixedBuffer &operator=(const FixedBuffer &) = delete;

        const char* data() const {return _data;};
        size_t length() const {return _cur - _data;}
        size_t remain() {return end() - _cur;}
        char *current() {return _cur;}
        /* 增加地址偏移量 与current()配合使用 */
        void offset(size_t off) {_cur += off;}
        void reset() {_cur = _data;}

        bool append(const char *buf,size_t len) {
            if (len > remain()) {
                return false;
            }
            memcpy(_cur,buf,len);
            offset(len);
            return true;
        }
    private:
        const char* end() const { return _data + sizeof _data; }
    private:
        char _data[bufsize]; // FIXME unicode?
        char *_cur = nullptr;
};
}
namespace log {
class LogStream {
    typedef LogStream self;
    public:
        static const size_t kMaxLogMessageLen = 3000;
        typedef internal::FixedBuffer<kMaxLogMessageLen> Buffer;

        /* 重载<<运算符 */
        self& operator<<(bool v) {
            _buf.append(v ? "1" : "0", 1);
            return *this;
        }
        self& operator<<(short v) {
            *this << static_cast<int>(v);
            return *this;
        }
        self& operator<<(unsigned short v) {
            *this << static_cast<unsigned int>(v);
            return *this;
        }
        self& operator<<(int);
        self& operator<<(unsigned int);
        self& operator<<(long);
        self& operator<<(unsigned long);
        self& operator<<(long long);
        self& operator<<(unsigned long long);

        self& operator<<(const void*);

        self& operator<<(float v) {
            *this << static_cast<double>(v);
            return *this;
        }
        self& operator<<(double);

        self& operator<<(char v) { 
            _buf.append(&v,1); 
            return *this;
        }
        self& operator<<(const unsigned char*str) {
            // 底层转换，不损失精度
            *this << reinterpret_cast<const char*>(str);
            return *this;
        }
        self& operator<<(const char* str) {
            if (str) {
                _buf.append(str,strlen(str));
            } else {
                _buf.append("(null)",6);
            }
            return *this;
        }

        self& operator<<(const std::string &str) {
            _buf.append(str.c_str(),str.length());
            return *this;
        }

        self& operator<<(const Buffer &buf) {
            _buf.append(buf.data(),buf.length());
            return *this;
        }
        
        Buffer& buffer(){return _buf;}
#ifdef UNITTEST
        std::string toString();
        void reset();
#endif
    private:
        /* 转换int并拷贝到buf中 */
        template<typename T> void copy2buf(T v);
    private:
        Buffer _buf;
    static const int maxNumericSize = 32;
};
}
}

#endif /* ifndef _YAMQ_LOG_LOGSTREAM */
