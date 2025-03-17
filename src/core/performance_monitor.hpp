#pragma once
#include <chrono>
#include <atomic>
#include <string>
#include <sstream>

namespace lwip {

class PerformanceMonitor {
public:
    static PerformanceMonitor& instance() {
        static PerformanceMonitor inst;
        return inst;
    }
    
    void record_throughput(size_t bytes) {
        total_bytes_ += bytes;
    }
    
    std::string get_performance_report() const {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_).count();
        if (duration == 0) { duration = 1; }
        std::stringstream ss;
        ss << "Performance Report:\n"
           << "Total bytes processed: " << total_bytes_ << "\n"
           << "Average throughput: " << (total_bytes_ / duration) << " bytes/s\n"
           << "Total operations: " << total_operations_ << "\n"
           << "Error count: " << error_count_ << "\n";
        return ss.str();
    }
    
private:
    PerformanceMonitor() : start_time_(std::chrono::steady_clock::now()) {}
    std::atomic<uint64_t> total_bytes_{0};
    std::atomic<uint64_t> total_operations_{0};
    std::atomic<uint64_t> error_count_{0};
    std::chrono::steady_clock::time_point start_time_;
};

} // namespace lwip