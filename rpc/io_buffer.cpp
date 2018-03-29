#include "rpc/io_buffer.h"
#include <assert.h>
#include <string.h>
#include <algorithm>
#include <sys/uio.h>
#include "log/logging.h"
#include <endian.h>

const int IoBuffer::kInitialSize;
const int IoBuffer::kCheapPrepend;

IoBuffer::IoBuffer(size_t initial_size):
    _buf(initial_size + kCheapPrepend),
    _reader_index(kCheapPrepend),
    _writer_index(_reader_index) 
{}

void IoBuffer::wseek(int offset) {
    assert(offset <= WritableBytes());
    _writer_index += offset;
    // TODO need compact ?
}

void IoBuffer::makeSpace(size_t n) {
    if (WritableBytes() + PrependBytes() < static_cast<int>(n) + kCheapPrepend) {
        // actual need realloc memory
        _buf.resize(_writer_index+n); // not always realloc, only when capacity isn't enough
    } else {
        // move data to release some memory
        compact(); // FIXME not efficient
    }
}

// 紧致结构 
inline void IoBuffer::compact() {
    assert(kCheapPrepend < _reader_index); // Never happen
    size_t readable_chars = ReadableBytes();
    std::copy(begin()+_reader_index,begin()+_writer_index,begin()+kCheapPrepend);
    _reader_index = kCheapPrepend;
    _writer_index = _reader_index + readable_chars;
}

void IoBuffer::Put(const char *data,size_t size) {
    // ensure enough write chars
    if (WritableBytes() < static_cast<int>(size)) {
        makeSpace(size);
    }
    std::copy(data,data+size,wpeek()); // memmove
    //::memcpy(begin(),data,size); // memcpy 和 memmove 的区别在于memmove可以完成地址重叠的拷贝
    wseek(size);
}

void IoBuffer::Append(const char *data,size_t size) {
    Put(data,size);
}

void IoBuffer::Prepend(const char *data,size_t size) {
    assert(size <= PrependChars());
    _reader_index -= size;
    std::copy(data,data+size,begin()+_reader_index);
}

void IoBuffer::Seek(int offset) {
    assert(offset <= ReadableBytes());
    if (offset < ReadableBytes()) {
        _reader_index += offset;
    } else {
        _reader_index = _writer_index = kCheapPrepend;
    }
}

int IoBuffer::Get(char *data,size_t size) {
    if (ReadableBytes() <= 0) {
        return 0;
    }
    size_t actual = size < ReadableBytes() ? size : ReadableBytes();
    std::copy(Peek(),Peek()+actual,data);
    Seek(actual);
    return actual;
}

size_t IoBuffer::Retrieve(int fd) {
    // saved an ioctl()/FIONREAD call to tell how much to read
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writeable = WritableBytes();
    vec[0].iov_base = wpeek();
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    const int iovcnt = (writeable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0) {
        LOG(FATAL) << "fail to readv fd";
    } else if (n <= static_cast<ssize_t>(writeable)) {
        wseek(n);
    } else {
        wseek(writeable);
        Put(extrabuf, n - writeable);
    }
    return n;
}

void IoBuffer::Swap(IoBuffer &buf) {
    _buf.swap(buf._buf);
    std::swap(_reader_index,buf._reader_index);
    std::swap(_writer_index,buf._writer_index);
}

void IoBuffer::Shrink(int reserve) {
    IoBuffer buf;
    reserve += ReadableBytes();
    if (buf.WritableBytes() < static_cast<int>(reserve)) {
        buf.makeSpace(reserve);
    }
    buf.Append(Peek(),ReadableBytes());     
    Swap(buf);
}

void IoBuffer::Reset() {
    IoBuffer buf;
    Swap(buf);
}

//int Put(const void *data,size_t size) {
//    return Put(static_cast<const char *>(data),size);
//}

//int Get(void *data, size_t size) {
//    return Get(static_cast<char *>(data),size);
//}

int IoBuffer::PutInt32(int32_t val) {
    val = be32toh(val); // trans to Big Endian
    Put((const char *)&val,sizeof val);
    return sizeof val;
}

int32_t IoBuffer::GetInt32() {
    int32_t val;
    int n = Get((char *)&val,sizeof val);
    assert(n == sizeof(val));
    val = htobe32(val); // trans to Little Endian
    return val;
}

int IoBuffer::PutString(const std::string &str) {
    Put(str.c_str(),str.length());
    return str.length();
}

std::string IoBuffer::GetAsString(int n) {
    assert(ReadableBytes() >= n);
    std::string str(Peek(),n);
    Seek(n);
    return str;
}

std::string IoBuffer::GetAllAsString() {
    return GetAsString(ReadableBytes());
}
