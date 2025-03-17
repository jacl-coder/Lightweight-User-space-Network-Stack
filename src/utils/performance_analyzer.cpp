#include "performance_analyzer.hpp"
#include <sstream>

namespace lwip {

PerformanceAnalyzer& PerformanceAnalyzer::instance() {
    static PerformanceAnalyzer instance;
    return instance;
}

void PerformanceAnalyzer::start_measurement(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    measurements_[name].start_time = std::chrono::steady_clock::now();
    measurements_[name].count++;
}

void PerformanceAnalyzer::end_measurement(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto& measurement = measurements_[name];
    auto duration = std::chrono::steady_clock::now() - measurement.start_time;
    measurement.total_time += std::chrono::duration<double>(duration).count();
}

std::string PerformanceAnalyzer::get_report() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::stringstream ss;
    ss << "Performance Report:\n";
    
    for (const auto& [name, data] : measurements_) {
        double avg_time = data.count > 0 ? data.total_time / data.count : 0;
        ss << name << ":\n"
           << "  Total calls: " << data.count << "\n"
           << "  Average time: " << avg_time * 1000 << "ms\n"
           << "  Total time: " << data.total_time << "s\n\n";
    }
    
    return ss.str();
}

} // namespace lwip
