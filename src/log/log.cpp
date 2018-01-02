#include "log/log.h"
#include "utilities.h"
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <iomanip>

using std::ostringstream;
using std::setw;
using std::setfill;
using namespace yamq::log;

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

    WallTime now = WallTime_Now();
    _data->timestamp = static_cast<time_t>(now);
	localtime_r(&_data->timestamp, &_data->tm_time);
	int usecs = static_cast<int>((now - _data->timestamp) * 1000000);

    _data->fullname = file;
    _data->basename = const_basename(file);
    _data->line = line;
    //_data->severity = severity;
    _data->flushed = false;
    _data->num_chars_to_log = 0;

	if (line != -1) {
		stream() << LogSeverityNames[severity][0]
			<< setw(2) << 1+_data->tm_time.tm_mon
			<< setw(2) << _data->tm_time.tm_mday
			<< ' '
			<< setw(2) << _data->tm_time.tm_hour  << ':'
			<< setw(2) << _data->tm_time.tm_min   << ':'
			<< setw(2) << _data->tm_time.tm_sec   << "."
			<< setw(6) << usecs
			<< ' '
			<< setfill(' ') << setw(5)
			<< static_cast<unsigned int>(GetTID()) << setfill('0')
			<< ' '
			<< _data->basename << ':' << _data->line << "] ";
	}
	_data->num_prefix_chars = _data->stream.pcount();
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
    _data->num_chars_to_log = _data->stream.pcount();

	bool append_newline =
		(_data->message_text[_data->num_chars_to_log-1] != '\n');
	char original_final_char = '\0';

	// If we do need to add a \n, we'll do it by violating the memory of the
	// ostrstream buffer.  This is quick, and we'll make sure to undo our
	// modification before anything else is done with the ostrstream.  It
	// would be preferable not to do things this way, but it seems to be
	// the best way to deal with this.
	if (append_newline) {
		original_final_char = _data->message_text[_data->num_chars_to_log];
		_data->message_text[_data->num_chars_to_log++] = '\n';
	} 

    // TODO Mutex Lock
    (this->*(_data->send_method))(); 
    LogDestination::WaitForSinks(_data);

	if (append_newline) {
		// Fix the ostrstream back how it was before we screwed with it.
		// It's 99.44% certain that we don't need to worry about doing this.
		_data->message_text[_data->num_chars_to_log-1] = original_final_char;
	}

    _data->flushed = true;
}

// will been import By Init
void LogMessage::SendToLog() {
    LogDestination::LogToAllLogfiles(_data->severity,
            _data->timestamp,
            _data->message_text,
            _data->num_chars_to_log);
}

LogDestination::LogDestination(LogSeverity severity,
        const char* base_filename)
    : _fileobject(severity, base_filename),
    _logger(&_fileobject) {
    }

inline void LogDestination::LogToAllLogfiles(LogSeverity severity,
        time_t timestamp,
        const char* message, size_t len) {
    int i = severity;
    for (;i >= 0; --i) {
        // 挨个写入文件
        I_DEBUG("i:%d\n",i);
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
    _severity(severity),
    _file(NULL) {
        assert(severity >= 0);
        assert(severity < NUM_SEVERITIES);
        assert(_file == NULL);
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

    int fd = open(filename,O_WRONLY | O_CREAT | O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0) {return false;}

    _file = fdopen(fd,"a");
    return true;
}

void LogFileObject::Write(bool force_flush,
        time_t timestamp,
        const char* message,
        int message_len) {

    I_DEBUG("log write: %s,len: %d,time:%d\n",message,message_len,timestamp);
    I_DEBUG("file is NULL: %d\n",_file==NULL);
    if (_file == NULL) {
        if (_base_filename.empty()) {
            // 生成默认文件名
            // webserver.examplehost.root.log.INFO.19990817-150000.4354
            string stripped_filename(GetProgramShortName());
            stripped_filename = stripped_filename + ".log." + 
                LogSeverityNames[_severity] + ".";
            string dir("."); // 当前目录
            _base_filename = dir + "/" + stripped_filename;

            I_DEBUG("_base_filename:%s\n",_base_filename.c_str());
        }
        I_DEBUG("_base_filename:%s\n",_base_filename.c_str());
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
        I_DEBUG("create log file.");
        if (CreateLogfile(time_pid_string)) {
            success = true;
        }

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
    }

    fwrite(message, 1, message_len, _file);
    _file_length += message_len;
}

void LogFileObject::Flush() {
}


void yamq::InitLog(const char* argv0) {
    InitLogUtilities(argv0);
}

void yamq::ShutdownLog() {
    ShutdownLogUtilities();
}


