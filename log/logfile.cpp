#include "log/logfile.h"
#include <stdlib.h>
#include <assert.h>

namespace yamq {
#ifdef SYNC_LOGGING
std::unique_ptr<LogFile> g_logfileptr;
#endif
namespace log {

LogFile::LogFile(const char *basename,const char *filename,uint32_t rollSize,const char *extension):
    _basename(basename),_filename(filename),_extension(extension),
    _lastTimestamp(0),_rollIndex(0),_writenChars(0),_kRollSize(rollSize) {
    LOG(TRACE) << "KRollSize:" << _kRollSize << ",writenChars:" << _writenChars;
    LOG(TRACE) << "Basename:" << _basename << ",filename:" << _filename;
    roll(true);
}

size_t LogFile::append(const char *data,size_t len) {
    size_t writen = 0;
    {
        std::lock_guard<std::mutex> guard(_mutex);
        writen = _file->write(data,len);
    }
    _writenChars += writen;
    //LOG(TRACE) << "Writen:" << writen << " total_write:" << _writenChars << " max_roll_size:" << _kRollSize;
    if (_writenChars >= _kRollSize) {
        roll(false);
    }
    return writen;
}

bool LogFile::roll(bool next) {
    ReadableTime now_tm;
    Timestamp now = nowTime(&now_tm);
    std::string filepath;

    if (!next) {
        // 超限roll
        LOG(TRACE) << "Start roll(false)";
        ++_rollIndex;
        filepath = _lastFilepath;
    } else {
        LOG(TRACE) << "Start roll(true)";
        // 构建logfile名称
        filepath += _basename;
        filepath += "/";
        filepath += _filename;
        filepath += ".";
        filepath += _extension;

        std::string linkpath(filepath);

        char timebuf[32];
        strftime(timebuf, sizeof timebuf, "%Y%m%d%H", &now_tm);
        filepath += ".";
        filepath += timebuf;
    }
    if (roll(filepath)) {
        _lastTimestamp = now;
        return true;
    }
    return false;
}

bool LogFile::roll(const std::string &path) {
    assert(!path.empty());

    std::string filepath(path);
    if (_rollIndex > 0) {
        filepath += "_";
        char num[32];
        snprintf(num,32,"%u",_rollIndex);
        filepath += num;
    }

    LOG(TRACE) << "Logfile:" << filepath;

    /* 创建新日志文件 */
    {
        std::lock_guard<std::mutex> guard(_mutex);
        auto fileptr = std::unique_ptr<internal::FileWrapper>(new internal::FileWrapper(filepath.c_str(),FILE_MODE));
        if (!fileptr->valid()) {
            fileptr.release();
            return false;
        }
        _file = std::move(fileptr);
    }

    _writenChars = 0;
    _lastFilepath = path;
    
    const char *dot = strrchr(filepath.c_str(),'.');
    assert(dot != NULL);

    std::string linkpath(filepath.c_str(),dot - filepath.c_str());
    // remove old link file
    unlink(linkpath.c_str());

    const char* slash = strrchr(filepath.c_str(), '/');
    const char *linkdest = slash ? (slash + 1) : filepath.c_str();

    LOG(TRACE) << "Linkdest:" << linkdest;
    if (symlink(linkdest, linkpath.c_str()) != 0) {
        // silently ignore failures
    }
    return true;
}

}
}
