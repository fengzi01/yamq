#ifndef _YAMQ_LOG_H
#define _YAMQ_LOG_H 
#include <iostream>
#include <vector>
#include <assert.h>
#include <time.h>
#include <string>
using std::string;

namespace yamq {
const int LOG_INFO = 0, LOG_WARNING = 1, LOG_ERROR = 2, LOG_FATAL = 3,
      NUM_SEVERITIES = 4;

#define LOG(severity) COMPACT_LOG_ ## severity.stream()
#define COMPACT_LOG_INFO yamq::log::LogMessage( \
    __FILE__, __LINE__)
#define COMPACT_LOG_WARNING yamq::log::LogMessage( \
    __FILE__, __LINE__, yamq::LOG_WARNING)

namespace log {
    typedef int LogSeverity;

    class LogStream;
    class LogSink;
    class Logger;

    // 其实就是个streambuf
    class LogStreamBuf : public std::streambuf {
        public:
        LogStreamBuf(char *buf, int len) {
            // 设置pointer,保留最后两个字符
            // setbase setend
            setp(buf, buf + len - 2);
        }

        // This effectively ignores overflow.
        virtual int_type overflow(int_type ch) {
            return ch;
        }
        // 缓冲区长度
        size_t pcount() const { return pptr() - pbase(); }
        // 缓冲区开始
        char* pbase() const { return std::streambuf::pbase(); }
    };

    /**
     * 消息类
     */
    class LogMessage {
        public:
        class LogStream : public std::ostream {
            public:
                LogStream(char *buf, int len, int ctr)
                    : std::ostream(NULL),
                    _streambuf(buf, len),
                    _ctr(ctr) {
                        // 指定stream写入的buf空间
                        rdbuf(&_streambuf);
                    }
                // Legacy std::streambuf methods.
                size_t pcount() const { return _streambuf.pcount(); }
                char* pbase() const { return _streambuf.pbase(); }
                char* str() const { return pbase(); }

            private:
                LogStreamBuf _streambuf;
                int _ctr;

            private:
                LogStream(const LogStream&);
                LogStream& operator=(const LogStream&);
        };
        public:
        struct LogMessageData;

        public:
        LogMessage();
        ~LogMessage();

        std::ostream& stream();
        LogMessage(const char* file, int line, LogSeverity severity);

        // Flush a buffered message to the sink set in the constructor.
        void Flush();

        // only passed as SendMethod arguments to other LogMessage methods:
        void SendToLog();
        static const size_t maxLogMessageLen = 30000;

        private: 
        void Init(const char* file, int line, LogSeverity severity,
                void (LogMessage::*send_method)());
        LogMessageData *_data;
        LogMessageData *_allocated;
    };

    


    class LogDestination {
        public:
            static void LogToAllLogfiles(LogSeverity severity,
                    time_t timestamp,
                    const char* message, size_t len);

            static void LogToSinks(LogSeverity severity,
                    const char *full_filename,
                    const char *base_filename,
                    int line,
                    time_t timestamp,
                    const char* message,
                    size_t message_len);

            static void WaitForSinks(LogMessage::LogMessageData *data);

            static void MaybeLogToLogfile(LogSeverity severity,
                    time_t timestamp,
                    const char* message, size_t len);
        private:

            LogDestination(LogSeverity severity, const char* base_filename);

            // 初始化
            static LogDestination* log_destination(LogSeverity severity);

        public:
            Logger *_logger;
            // 静态成员
            static LogDestination* _log_destinations[NUM_SEVERITIES];
            static std::vector<LogSink *> &_sinks;
    };

    class LogSink {
        public:
            virtual void send() = 0;
    };

    class Logger {
        public:
            virtual void Write(bool force_flush,
                    time_t timestamp,
                    const char* message,
                    int message_len) = 0;
            virtual void Flush() = 0;
    };

    class LogFileObject : public Logger {
        public:
            LogFileObject(LogSeverity severity, const char* base_filename);
            ~LogFileObject();

            virtual void Write(bool force_flush,
                    time_t timestamp,
                    const char* message,
                    int message_len) = 0;
            virtual void Flush() = 0;

        private:
            string _base_filename;
            FILE *_file;
            LogSeverity _severity;
            unsigned int _file_length;

            bool CreateLogfile(const string& time_pid_string);
    };

    extern Logger* GetLogger(LogSeverity level);
    extern void SetLogger(LogSeverity level, Logger* logger);

    class NullStream : public LogMessage::LogStream {
        public:
            // Initialize the LogStream so the messages can be written somewhere
            // (they'll never be actually displayed). This will be needed if a
            // NullStream& is implicitly converted to LogStream&, in which case
            // the overloaded NullStream::operator<< will not be invoked.
            NullStream &stream() { return *this; }
        private:
            // A very short buffer for messages (which we discard anyway). This
            // will be needed if NullStream& converted to LogStream& (e.g. as a
            // result of a conditional expression).
            char _msg_buf[2];
    };
} //log
}//yamq
namespace {
void InitLog(const char* argv0);

void ShutdownLog();

} // namespace
#endif /* ifndef _YAMQ_LOG_H */
