#include "log/logfile.h"
#include "log/utilities.h"
#include "log/logging.h"

namespace yamq {
namespace log {

LogFile::LogFile(const char *basename,const char *filename,uint32_t rollsize = kMaxRollSize):_basename(basename),_filename(filename),_rollsize(rollsize),_extension("log"),_writenChars(0),_lastHour(-1) {
    //roll();
}

size_t LogFile::append(const char *data,size_t len) {
    size_t writen = 0;
    roll();
    writen = _file->write(data,len);
    _writenChars += writen;
    return writen;
}

bool LogFile::roll() {
    Timestamp now = nowTime();
    time_t time = static_cast<time_t>(now/1000000);
    struct tm tm;
    gmtime_r(&time, &tm);
    if (_lastHour == tm.tm_hour) {
        return false;
    }
    _lastHour = tm.tm_hour;
    // FIXME kMaxRollSize?

    // 构建logfile名称
    std::string filepath(_basename);
    filepath += "/";
    filepath += _filename;
    filepath += ".";
    filepath += _extension;

    std::string linkpath(filepath);

    char timebuf[32];
    strftime(timebuf, sizeof timebuf, ".%Y%m%d%H%M%S.", &tm);
    filepath += ".";
    filepath += timebuf;

    LOG(INFO) << "Logfile:" << filepath;

    /* 创建新日志文件 */
    auto fileptr = std::unique_ptr<internal::FileWrapper>(new internal::FileWrapper(filepath.c_str(),FILE_MODE));
    if (!fileptr->valid()) {
        fileptr.release();
        return false;
    }
    _file = std::move(fileptr);
    // 置零
    _writenChars = 0;
    
    // 创建链接文件
    unlink(linkpath.c_str());
    const char* slash = strrchr(filepath.c_str(), '/');
    const char *linkdest = slash ? (slash + 1) : (_filename+_extension).c_str();

    LOG(INFO) << "Linkdest:" << linkdest;

    if (symlink(linkdest, linkpath.c_str()) != 0) {
        // silently ignore failures
    }
    return true;
}

}
}
