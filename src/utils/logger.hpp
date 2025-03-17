#pragma once
#include <iostream>
#include <string>
#include <mutex>

namespace lwip {

enum class LogLevel {
    INFO,
    WARN,    // 改为WARN，保持一致性
    ERROR,
    DEBUG
};

class Logger {
public:
    static Logger& instance();
    void log(LogLevel level, const std::string& message);
    void set_level(LogLevel level) { min_level_ = level; }

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel min_level_{LogLevel::INFO};
    std::mutex mutex_;
};

#define LOG_INFO(msg)  lwip::Logger::instance().log(lwip::LogLevel::INFO, msg)
#define LOG_ERROR(msg) lwip::Logger::instance().log(lwip::LogLevel::ERROR, msg)
#define LOG_WARN(msg)  lwip::Logger::instance().log(lwip::LogLevel::WARN, msg)
#define LOG_DEBUG(msg) lwip::Logger::instance().log(lwip::LogLevel::DEBUG, msg)

} // namespace lwip
