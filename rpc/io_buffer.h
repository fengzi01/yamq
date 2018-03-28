#ifndef _IO_BUFFER_H
#define _IO_BUFFER_H
#include <vector>
/* io读取应用层buffer类 */
class IoBuffer {
    public:
        static const int kInitialSize = 1024;
        static const int kCheapPrepend = 8;
    public:
        explicit IoBuffer(size_t initial_size);
        void Append(const char *data,size_t size);
        void Prepend(const char *data,size_t size);
        /* peek read pointer */
        char *Peek() { return begin() + _read_index; } 
        /* set read index via offset */
        void Seek(int offset);
        /* retrieve from fd,e.g.:socket fd */
        size_t Retrieve(int fd);

        void Shrink();

        int WriteableChars() { return _buf.size() - _write_index; }
        int PrependChars() { return _read_index; }
        int ReadableChars() { return _write_index - _read_index; } 

        void Swap(IoBuffer &buf);
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
