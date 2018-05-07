#ifndef _IO_BUFFER_H
#define _IO_BUFFER_H
#include <stddef.h>
#include <vector>
#include <string>
#include <algorithm>

/* io读取应用层buffer类 */
class IoBuffer {
    public:
        static const int kInitialSize = 1024;
        static const int kCheapPrepend = 8;
    public:
        explicit IoBuffer(size_t initial_size = kInitialSize);

        void Put(const char *data,size_t size);
        void Append(const char *data,size_t size);
        void Append(std::string str) {
            Append(str.c_str(),str.length());
        }
        void Prepend(const char *data,size_t size);

        /* peek read pointer */
        const char *Peek() const { return _buf.data() + _reader_index; } 
        /* set read index via offset */
        void Seek(int offset);

        // search
        const char* findCRLF() const
        {
            // FIXME: replace with memmem()?
            const char *start = Peek();
            const char *end = start + _writer_index;
            const char* crlf = std::search(start, end, kCRLF, kCRLF+2);
            return crlf == end ? NULL : crlf;
        }

        int Get(char *data,size_t size);
        void GetUntil(const char *end) {
            Seek(end - Peek());
        }

        /* retrieve from fd,e.g.:socket fd */
        size_t Retrieve(int fd);

        void Swap(IoBuffer &buf);
        void Shrink(int reserve);
        void Reset();

//        int Put(const void *data,size_t size);
//        int Get(void *data,size_t size);
        
        int PutChar(char val);
        char GetChar();

        int PutInt16(int16_t val);
        int16_t GetInt16();

        int PutInt32(int32_t val);
        int32_t GetInt32();

        int PutInt64(int64_t val);
        int64_t GetInt64();

        int PrependInt16();
        int PrependInt32();
        int PrependInt64();

        int PutString(const std::string &str);
        std::string GetAsString(int n);

        std::string GetAllAsString();

        int WritableBytes() { return _buf.size() - _writer_index; }
        int PrependBytes() { return _reader_index; }
        int ReadableBytes() { return _writer_index - _reader_index; } 

        size_t Capacity() { return _buf.capacity(); }
        size_t Size() { return _buf.size(); }
    private:
        char *begin() { return _buf.data(); } // c++11 
        char *wpeek() { return begin() + _writer_index; }
        void wseek(int offset);
        void makeSpace(size_t n);
        void compact();
    private:
        std::vector<char> _buf;
        int _reader_index;
        int _writer_index;

        static const char kCRLF[];
};
#endif /* _IO_BUFFER_H */
