#include "log/logstream.h"
#include <algorithm> // std::reverse

namespace yamq {
namespace internal {
const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
static_assert(sizeof(digits) == 20, "");

const char digitsHex[] = "0123456789ABCDEF";
static_assert(sizeof digitsHex == 17, "");

// Efficient Integer to String Conversions, by Matthew Wilson.
// FIXME How ?
template<typename T> size_t convert(char buf[], T value)
{
    T i = value;
    char* p = buf;
    do
    {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0)
    {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
}

// ptr to string
size_t convertHex(char buf[], uintptr_t value)
{
  uintptr_t i = value;
  char* p = buf;

  do
  {
    int lsd = static_cast<int>(i % 16);
    i /= 16;
    *p++ = digitsHex[lsd];
  } while (i != 0);

  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}
} // yamq::internal

namespace log {
template<typename T> void LogStream::copy2buf(T v) {
    if (_buf.remain() >= maxNumericSize) {
        size_t len = internal::convert(_buf.current(),v);
        _buf.offset(len);
    }
}

LogStream& LogStream::operator<<(int v) {
    copy2buf(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned int v) {
    copy2buf(v);
    return *this;
}

LogStream& LogStream::operator<<(long v) {
    copy2buf(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned long v) {
    copy2buf(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v) {
    copy2buf(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned long long v) {
    copy2buf(v);
    return *this;
}

LogStream& LogStream::operator<<(const void *p) {
    if (_buf.remain() > maxNumericSize + 2) {
        uintptr_t v = reinterpret_cast<uintptr_t>(p);
        _buf.append("0x",2);
        size_t len = internal::convertHex(_buf.current(),v);
        _buf.offset(len);
    }
    return *this;
}

// FIXME
LogStream& LogStream::operator<<(double v)
{
  if (_buf.remain() >= maxNumericSize)
  {
    size_t len = snprintf(_buf.current(), maxNumericSize, "%.12g", v);
    _buf.offset(len);
  }
  return *this;
}


} // yamq::log
} // yamq
