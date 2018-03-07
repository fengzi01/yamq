#include "log/logfile.h"
#include <stdlib.h>
#include <assert.h>

namespace yamq {
namespace log {

LogFile::LogFile(const std::string &dirname,const std::string &filename,uint64_t rollSize,const char *extension):
    _dirname(dirname),_filename(filename),_extension(extension),
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
    if (_filepath.empty()) {
        // 构建logfile名称
        _filepath += _dirname;
        _filepath += "/";
        _filepath += _filename;
        _filepath += ".";
        _filepath += _extension;
    }
    std::string filepath(_filepath);

    if (_rollIndex > 0) {
        filepath += "_";
        char num[32];
        snprintf(num,32,"%u",_rollIndex);
        filepath += num;
    }

    // start roll
    auto fileptr = std::unique_ptr<AppendFileWrapper>(new AppendFileWrapper(filepath));
    if (!fileptr->valid()) {
        fileptr.release();
        fprintf(stderr,"roll fail!\n");
        return false;
    }
    _file = std::move(fileptr);

    ++_rollIndex;
    _lastTimestamp = now;
    // }
    return true;
}

}
}
