#pragma once
#include <chrono>
#include <queue>
#include "tcp_packet.hpp"

namespace lwip {

class TCPRetransmitter {
public:
    void add_segment(const TCPPacket& packet);
    void handle_ack(uint32_t ack_number);
    void check_timeouts();
    
private:
    static constexpr auto RTO_MIN = std::chrono::milliseconds(200);
    static constexpr auto RTO_MAX = std::chrono::seconds(120);
    
    double srtt_{0};    // 平滑往返时间
    double rttvar_{0};  // 往返时间变化
    double rto_{1000};  // 重传超时
};

} // namespace lwip
