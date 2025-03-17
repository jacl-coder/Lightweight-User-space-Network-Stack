#include "logger.hpp"
#include <iostream>
#include <ctime>

namespace lwip {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < min_level_) return;

    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    const char* level_str = "";
    switch (level) {
        case LogLevel::DEBUG: level_str = "DEBUG"; break;
        case LogLevel::INFO:  level_str = "INFO"; break;
        case LogLevel::WARN:  level_str = "WARN"; break;
        case LogLevel::ERROR: level_str = "ERROR"; break;
    }
    
    std::cout << "[" << std::ctime(&time) << "][" << level_str << "] " 
              << message << std::endl;
}

} // namespace lwip
