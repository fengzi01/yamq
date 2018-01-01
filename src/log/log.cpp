#include "log/log.h"
#include "utilities.h"
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <iomanip>

using std::ostringstream;
using std::setw;
using namespace yamq::log;

struct LogMessage::LogMessageData {
    // LogMessageData();
    LogMessageData()
        : stream(message_text, LogMessage::maxLogMessageLen+1, 0) {
        }
    char message_text[LogMessage::maxLogMessageLen+1];
    int nums_chars_to_log;

    LogStream stream;

    LogSeverity severity; 

    void (LogMessage::*send_method)();

    
    time_t timestamp;

    const char* basename;
    const char* fullname;
    int line;

    bool flushed;
    bool first_fatal;
};

LogMessage::LogMessage(const char* file, int line, LogSeverity severity) {
    Init(file, line, severity, &LogMessage::SendToLog);
}

// Init
void LogMessage::Init(const char* file,
                        int line,
                        LogSeverity severity,
                        void (LogMessage::*send_method)()) {
    // 初始化data
    _allocated = new LogMessageData;
    _data = _allocated;

    stream().fill('0');

    _data->send_method = send_method;
    // 时间
    WallTime now = WallTime_Now();
    _data->timestamp = static_cast<time_t>(now);

    _data->fullname = file;
    _data->basename = const_basename(file);
    _data->line = line;
    _data->flushed = false;

    _data->nums_chars_to_log = 0;
}
// 析构函数中Flush输出到文件中
LogMessage::~LogMessage() {
    Flush();
    delete _allocated;
}
// ret data::stream
std::ostream& LogMessage::stream() {
    return _data->stream;
}
void LogMessage::Flush() {
    if (_data->flushed) {
        return;
    }

    // 字符串长度
    _data->nums_chars_to_log = _data->stream.pcount();

    // do flush
    // TODO Mutex Lock
    (this->*(_data->send_method))(); 
    LogDestination::WaitForSinks(_data);

    _data->flushed = true;
}

LogDestination::LogDestination(LogSeverity severity,
        const char* base_filename)
    : _fileobject(severity, base_filename),
    _logger(&_fileobject) {
    }

// will been import By Init
void LogMessage::SendToLog() {
    LogDestination::LogToAllLogfiles(_data->severity,
            _data->timestamp,
            _data->message_text,
            _data->nums_chars_to_log);
}

inline void LogDestination::LogToAllLogfiles(LogSeverity severity,
        time_t timestamp,
        const char* message, size_t len) {
    int i = severity;
    for (;i >= 0; --i) {
        // 挨个写入文件
        LogDestination::MaybeLogToLogfile(i,timestamp,message,len);
    }
}

inline void LogDestination::MaybeLogToLogfile(LogSeverity severity,
        time_t timestamp,
        const char* message, size_t len) {
    LogDestination *desination = LogDestination::log_destination(severity);
    desination->_logger->Write(true,timestamp,message,len);
}

// 静态成员需要单独定义
LogDestination* LogDestination::_log_destinations[NUM_SEVERITIES];

inline LogDestination* LogDestination::log_destination(LogSeverity severity) {
    assert(severity >=0 && severity < NUM_SEVERITIES);
    if (!_log_destinations[severity]) {
        _log_destinations[severity] = new LogDestination(severity, NULL);
    }
    return _log_destinations[severity];
}

void LogDestination::WaitForSinks(LogMessage::LogMessageData *data) {
}

LogFileObject::LogFileObject(LogSeverity severity, const char* base_filename):
    _base_filename((base_filename != NULL) ? base_filename : ""),
    _severity(severity) {
        assert(severity >= 0);
        assert(severity < NUM_SEVERITIES);
}

LogFileObject::~LogFileObject() {
    if (NULL != _file) {
        fclose(_file);
        _file = NULL;
    }
}

bool LogFileObject::CreateLogfile(const string &time_pid_string) {
    string string_filename = _base_filename+time_pid_string;
    const char *filename = string_filename.c_str();

    printf("filename:%s\n",filename);

    int fd = open(filename,O_WRONLY | O_CREAT | O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0) {return false;}

    _file = fdopen(fd,"a");

    return true;
}

void LogFileObject::Write(bool force_flush,
        time_t timestamp,
        const char* message,
        int message_len) {
    bool success = false;
    ostringstream time_pid_stream;
    struct ::tm tm_time;
    localtime_r(&timestamp, &tm_time);
    time_pid_stream.fill('0');
    time_pid_stream << 1900+tm_time.tm_year
        << setw(2) << 1+tm_time.tm_mon
        << setw(2) << tm_time.tm_mday
        << '-'
        << setw(2) << tm_time.tm_hour
        << setw(2) << tm_time.tm_min
        << setw(2) << tm_time.tm_sec
        << '.'
        << getpid();
    time_pid_stream.fill('0');
    const string& time_pid_string = time_pid_stream.str();
    if (CreateLogfile(time_pid_string)) {
        success = true;
    }

    string stripped_filename;
    string dir("tmp");
    _base_filename = dir + "/" + stripped_filename;

    printf("base_filename:%s\n",_base_filename.c_str());

    ostringstream file_header_stream;
    file_header_stream.fill('0');
    file_header_stream << "Log file created at: "
        << 1900+tm_time.tm_year << '/'
        << setw(2) << 1+tm_time.tm_mon << '/'
        << setw(2) << tm_time.tm_mday
        << ' '
        << setw(2) << tm_time.tm_hour << ':'
        << setw(2) << tm_time.tm_min << ':'
        << setw(2) << tm_time.tm_sec << '\n'
        << "Running on machine: "
        //<< LogDestination::hostname() << '\n'
        << "xxx" << "\n"
        << "Log line format: [IWEF]mmdd hh:mm:ss.uuuuuu "
        << "threadid file:line] msg" << '\n';
    const string& file_header_string = file_header_stream.str();

    const int header_len = file_header_string.size();
    fwrite(file_header_string.data(), 1, header_len, _file);
    _file_length += header_len;

    fwrite(message, 1, message_len, _file);
    _file_length += message_len;
}

void LogFileObject::Flush() {
}


