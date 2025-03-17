#include "performance_monitor.hpp"
#include <sstream>

namespace lwip {

PerformanceMonitor& PerformanceMonitor::instance() {
    static PerformanceMonitor instance;
    return instance;
}

void PerformanceMonitor::record_latency([[maybe_unused]] const std::string& operation, [[maybe_unused]] std::chrono::microseconds latency) {
    total_operations_++;
    // TODO: 实现延迟统计
}

void PerformanceMonitor::record_throughput(size_t bytes) {
    total_bytes_ += bytes;
}

void PerformanceMonitor::record_error([[maybe_unused]] const std::string& type) {
    error_count_++;
}

std::string PerformanceMonitor::get_performance_report() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_).count();
    
    std::stringstream ss;
    ss << "Performance Report:\n"
       << "Total bytes processed: " << total_bytes_ << "\n"
       << "Average throughput: " << (total_bytes_ / (duration ? duration : 1)) << " bytes/s\n"
       << "Total operations: " << total_operations_ << "\n"
       << "Error count: " << error_count_ << "\n";
    
    return ss.str();
}

} // namespace lwip
