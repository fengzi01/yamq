#include "log/logfile.h"
#include <stdlib.h>
#include <assert.h>

namespace yamq {
namespace log {

LogFile::LogFile(const char *basename,const char *filename,uint64_t rollSize,const char *extension):
    _basename(basename),_filename(filename),_extension(extension),
    _lastTimestamp(0),_rollIndex(0),_kRollSize(rollSize) {
    roll();
}

size_t LogFile::append(const char *data,size_t len) {
    size_t writen = 0;
    {
        std::lock_guard<std::mutex> guard(_mutex);
        writen = _file->append(data,len);
        if (_kRollSize > 0 && _file->writenChars() >= _kRollSize) {
            roll();
        }
    }
    return writen;
}

bool LogFile::roll() {
    fprintf(stderr,"start roll!\n");
    ReadableTime now_tm;
    Timestamp now = nowTime(&now_tm);

    // {
    // std::lock_guard<std::mutex> guard(_mutex);
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
    auto fileptr = std::unique_ptr<AppendFileWrapper>(new AppendFileWrapper(filepath.c_str(),FILE_MODE));
    if (!fileptr->valid()) {
        fileptr.release();
        return false;
    }
    _file.reset(fileptr.release());

    ++_rollIndex;
    _lastTimestamp = now;
    // }
    return true;
}

}
}
