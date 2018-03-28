#include "rpc/io_buffer.h"
#include <assert.h>
#include <string.h>
#include <algorithm>
#include <sys/uio.h>
#include "log/logging.h"

const int IoBuffer::kInitialSize;
const int IoBuffer::kCheapPrepend;

IoBuffer::IoBuffer(size_t initial_size):
    _buf(initial_size + kCheapPrepend),
    _read_index(kCheapPrepend),
    _write_index(_read_index) 
{}

void IoBuffer::wseek(int offset) {
    assert(offset <= WriteableChars());
    _write_index += offset;
}

void IoBuffer::makeSpace(size_t n) {
    if (WriteableChars() + PrependChars() < static_cast<int>(n) + kCheapPrepend) {
        // actual need realloc memory
        _buf.resize(_write_index+n); // not always realloc, only when capacity isn't enough
    } else {
        // move data to release some memory
        format(); // FIXME not efficient
    }
}

inline void IoBuffer::format() {
    assert(kCheapPrepend < _read_index); // Never happen
    size_t readable_chars = ReadableChars();
    std::copy(begin()+_read_index,begin()+_write_index,begin()+kCheapPrepend);
    _read_index = kCheapPrepend;
    _write_index = _read_index + readable_chars;
}

void IoBuffer::Append(const char *data,size_t size) {
    // ensure enough write chars
    if (WriteableChars() < static_cast<int>(size)) {
        makeSpace(size);
    }
    std::copy(data,data+size,begin()+_write_index); // memmove
    //::memcpy(begin(),data,size); // memcpy 和 memmove 的区别在于memmove可以完成地址重叠的拷贝
    _write_index += size;
}

void IoBuffer::Prepend(const char *data,size_t size) {
    assert(size <= PrependChars());
    _read_index -= size;
    std::copy(data,data+size,begin()+_read_index);
}

void IoBuffer::Seek(int offset) {
    assert(offset <= ReadableChars());
    if (offset < ReadableChars()) {
        _read_index += offset;
    } else {
        _read_index = _write_index = kCheapPrepend;
    }
}

size_t IoBuffer::Retrieve(int fd) {
    // saved an ioctl()/FIONREAD call to tell how much to read
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writeable = WriteableChars();
    vec[0].iov_base = wpeek();
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    const int iovcnt = (writeable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        LOG(FATAL) << "fail to readv fd";
    }
    else if (n <= writeable)
    {
        wseek(n);
    }
    else
    {
        wseek(writeable);
        Append(extrabuf, n - writeable);
    }
    return n;
}

void IoBuffer::Shrink(int reserve) {
    IoBuffer buf;
    reserve += ReadableChars();
    if (buf.WriteableChars() < static_cast<int>(reserve)) {
        buf.makeSpace(reserve);
    }
    buf.Append(Peek(),ReadableChars());     
    Swap(buf);
}

void IoBuffer::Swap(IoBuffer &buf) {
    _buf.swap(buf._buf);
    std::swap(_read_index,buf._read_index);
    std::swap(_write_index,buf._write_index);
}
