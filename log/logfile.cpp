#include "log/logfile.h"
#include <stdlib.h>
#include <assert.h>

namespace yamq {
namespace log {

LogFile::LogFile(const char *basename,const char *filename,uint32_t rollSize,const char *extension):
    _basename(basename),_filename(filename),_extension(extension),
    _lastTimestamp(0),_rollIndex(0),_writenChars(0),_kRollSize(rollSize) {
    roll();
}

size_t LogFile::append(const char *data,size_t len) {
    size_t writen = 0;
    {
        std::lock_guard<std::mutex> guard(_mutex);
        writen = _file->write(data,len);
    }
    _writenChars += writen;
    if (_kRollSize > 0 && _writenChars >= _kRollSize) {
        roll();
        _writenChars = 0;
    }
    return writen;
}

bool LogFile::roll() {
    fprintf(stderr,"start roll!\n");
    ReadableTime now_tm;
    Timestamp now = nowTime(&now_tm);

    {
    std::lock_guard<std::mutex> guard(_mutex);
    if (_lastFilepath.empty()) {
        // 构建logfile名称
        _lastFilepath += _basename;
        _lastFilepath += "/";
        _lastFilepath += _filename;
        _lastFilepath += ".";
        _lastFilepath += _extension;
    }
    std::string filepath(_lastFilepath);

    if (_rollIndex > 0) {
        filepath += "_";
        char num[32];
        snprintf(num,32,"%u",_rollIndex);
        filepath += num;
    }

    // start roll
    auto fileptr = std::unique_ptr<internal::FileWrapper>(new internal::FileWrapper(filepath.c_str(),FILE_MODE));
    if (!fileptr->valid()) {
        fileptr.release();
        return false;
    }

    _file = std::move(fileptr);
    ++_rollIndex;
    _lastTimestamp = now;
    }
    return true;
}

}
}
