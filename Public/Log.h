//
// Created by Yaozy on 2020/9/18.
//

#ifndef LOG_H
#define LOG_H

#include <string>
#include <mutex>
#include <thread>
#include <fstream>
#include <sstream>
#include <iostream>

#define DEBUG Base::Logger(Base::LOGLEVEL::debug).stream(__FILE__,__LINE__,__FUNCTION__)
#define INFO Base::Logger(Base::LOGLEVEL::info).stream(__FILE__,__LINE__,__FUNCTION__)
#define WARN Base::Logger(Base::LOGLEVEL::warn).stream(__FILE__,__LINE__,__FUNCTION__)
#define ERROR Base::Logger(Base::LOGLEVEL::error).stream(__FILE__,__LINE__,__FUNCTION__)
#define TRACE Base::Logger(Base::LOGLEVEL::trace).stream(__FILE__,__LINE__,__FUNCTION__)

namespace Base {
    enum class LOGLEVEL {
        error = 0,
        warn = 1,
        info = 2,
        debug = 3,
        trace = 4
    };
    class Logger {
    public:
        explicit Logger(LOGLEVEL level = LOGLEVEL::info);

        ~Logger()  {
            std::cout << _stream.str() << std::endl ;
        };

        Logger(const Logger &) = delete;

        Logger &operator=(const Logger &) = delete;

        template<typename T>
        inline Logger &operator<<(const T &info) {
            _stream << info;
            return *this;
        };

        Logger &stream(const std::string &file, std::size_t line, const std::string &func);
    public:

    private:
        LOGLEVEL _level;
        std::ostringstream _stream;
    };


    /*class FileLogger : public ILog, public IndependentThread<std::string> {

    public:

        explicit FileLogger(Logger::LOGLEVEL level = Logger::LOGLEVEL::info);

        ~FileLogger() override;

        FileLogger(const FileLogger &) = delete;

        FileLogger &operator=(const FileLogger &) = delete;

        void Trace(const std::string &msg, const std::string &file, std::size_t line, const std::string &func) override;

        void Debug(const std::string &msg, const std::string &file, std::size_t line, const std::string &func) override;

        void Info(const std::string &msg, const std::string &file, std::size_t line, const std::string &func) override;

        void Warn(const std::string &msg, const std::string &file, std::size_t line, const std::string &func) override;

        void Error(const std::string &msg, const std::string &file, std::size_t line, const std::string &func) override;

        void ReSetLogLevel(ILog::LOGLEVEL level) override;

        void SetLogRollover(const int &day);

    private:
        ILog::LOGLEVEL level;
        std::ofstream fileStream;

        void Run(std::string &msg) override;

        std::string logName;
        std::string sevenDaysAgoLogName;
        int offsetDay;
        bool rollover;
    };

    static std::string GetTime();

    static std::string GetDate(const int &offset);

    static bool FileExists(const std::string &name);

    void LogInit();

    void LogInit(const std::string &path);

    void Debug(const std::string &msg, const std::string &file, std::size_t line, const std::string &func);

    void Trace(const std::string &msg, const std::string &file, std::size_t line, const std::string &func);

    void Info(const std::string &msg, const std::string &file, std::size_t line, const std::string &func);

    void Warn(const std::string &msg, const std::string &file, std::size_t line, const std::string &func);

    void Error(const std::string &msg, const std::string &file, std::size_t line, const std::string &func);

    void SetLogPath(const std::string &path);

    void SetLevel(ILog::LOGLEVEL level);

    void SetLogRollover(const int &day);*/
}


#endif //LOG_H
