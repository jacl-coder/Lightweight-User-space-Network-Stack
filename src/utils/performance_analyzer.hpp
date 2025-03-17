#pragma once
#include <chrono>
#include <string>
#include <map>
#include <mutex>

namespace lwip {

class PerformanceAnalyzer {
public:
    static PerformanceAnalyzer& instance();
    
    void start_measurement(const std::string& name);
    void end_measurement(const std::string& name);
    std::string get_report() const;
    
private:
    struct Measurement {
        std::chrono::steady_clock::time_point start_time;
        double total_time{0};
        int64_t count{0};
    };
    
    std::map<std::string, Measurement> measurements_;
    mutable std::mutex mutex_;
};

#define MEASURE_SCOPE(name) \
    ScopedMeasurement measure##__LINE__(name)

class ScopedMeasurement {
public:
    explicit ScopedMeasurement(const std::string& name) 
        : name_(name) {
        PerformanceAnalyzer::instance().start_measurement(name);
    }
    
    ~ScopedMeasurement() {
        PerformanceAnalyzer::instance().end_measurement(name_);
    }
    
private:
    std::string name_;
};

} // namespace lwip
