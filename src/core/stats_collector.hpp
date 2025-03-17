#pragma once
#include <atomic>
#include <chrono>

namespace lwip {

class StatsCollector {
public:
    static StatsCollector& instance();

    void increment_bytes_sent(size_t bytes) { bytes_sent_ += bytes; }
    void increment_bytes_received(size_t bytes) { bytes_received_ += bytes; }
    void increment_packets_dropped() { packets_dropped_++; }
    void increment_tcp_connections() { active_tcp_connections_++; }
    void decrement_tcp_connections() { active_tcp_connections_--; }

    uint64_t get_bytes_sent() const { return bytes_sent_; }
    uint64_t get_bytes_received() const { return bytes_received_; }
    uint32_t get_packets_dropped() const { return packets_dropped_; }
    uint32_t get_active_tcp_connections() const { return active_tcp_connections_; }

private:
    std::atomic<uint64_t> bytes_sent_{0};
    std::atomic<uint64_t> bytes_received_{0};
    std::atomic<uint32_t> packets_dropped_{0};
    std::atomic<uint32_t> active_tcp_connections_{0};
};

} // namespace lwip
