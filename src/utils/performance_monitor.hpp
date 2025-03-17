#pragma once
#include <atomic>
#include <chrono>
#include <string>

namespace lwip {

class PerformanceMonitor {
public:
    static PerformanceMonitor& instance();
    
    void record_latency(const std::string& operation, std::chrono::microseconds latency);
    void record_throughput(size_t bytes);
    void record_error(const std::string& type);
    
    std::string get_performance_report() const;
    
private:
    std::atomic<uint64_t> total_bytes_{0};
    std::atomic<uint64_t> total_operations_{0};
    std::atomic<uint64_t> error_count_{0};
    std::chrono::steady_clock::time_point start_time_{std::chrono::steady_clock::now()};
};

} // namespace lwip
