#pragma once
#include <string>
#include <sstream>
#include <mutex>

namespace lwip {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static Logger& instance();
    
    void log(LogLevel level, const std::string& message);
    void set_level(LogLevel level) { min_level_ = level; }

private:
    Logger() = default;
    LogLevel min_level_{LogLevel::INFO};
    std::mutex mutex_;
};

#define LOG_DEBUG(msg) Logger::instance().log(LogLevel::DEBUG, msg)
#define LOG_INFO(msg)  Logger::instance().log(LogLevel::INFO, msg)
#define LOG_WARN(msg)  Logger::instance().log(LogLevel::WARN, msg)
#define LOG_ERROR(msg) Logger::instance().log(LogLevel::ERROR, msg)

} // namespace lwip
