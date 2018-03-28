#ifndef _IO_BUFFER_H
#define _IO_BUFFER_H
#include <stddef.h>
#include <vector>
/* io读取应用层buffer类 */
class IoBuffer {
    public:
        static const int kInitialSize = 1024;
        static const int kCheapPrepend = 8;
    public:
        explicit IoBuffer(size_t initial_size = kInitialSize);
        void Append(const char *data,size_t size);
        void Prepend(const char *data,size_t size);
        /* peek read pointer */
        char *Peek() { return begin() + _read_index; } 
        /* set read index via offset */
        void Seek(int offset);
        /* retrieve from fd,e.g.:socket fd */
        size_t Retrieve(int fd);

        void Shrink(int );

        int WriteableChars() { return _buf.size() - _write_index; }
        int PrependChars() { return _read_index; }
        int ReadableChars() { return _write_index - _read_index; } 

        void Swap(IoBuffer &buf);

        size_t Capacity() { return _buf.capacity(); }
        size_t Size() { return _buf.size(); }

    private:
        char *begin() { return _buf.data(); } // c++11 
        char *wpeek() { return begin() + _write_index; }
        void wseek(int offset);
        void makeSpace(size_t n);
        void format();
    private:
        std::vector<char> _buf;
        int _read_index;
        int _write_index;
};
#endif /* _IO_BUFFER_H */
