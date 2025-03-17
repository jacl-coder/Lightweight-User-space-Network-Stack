#include "stats_collector.hpp"
#include <sstream>

namespace lwip {

StatsCollector& StatsCollector::instance() {
    static StatsCollector instance;
    return instance;
}

std::string StatsCollector::get_stats_report() const {
    std::stringstream ss;
    ss << "Network Statistics:\n"
       << "Bytes sent: " << bytes_sent_ << "\n"
       << "Bytes received: " << bytes_received_ << "\n"
       << "Packets dropped: " << packets_dropped_ << "\n"
       << "Active TCP connections: " << active_tcp_connections_ << "\n"
       << "Total throughput: " << (bytes_sent_ + bytes_received_) << " bytes\n";
    return ss.str();
}

} // namespace lwip
