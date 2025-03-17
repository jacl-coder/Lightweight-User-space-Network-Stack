#pragma once
#include <chrono>
#include <list>
#include <functional>
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

    // 添加 Segment 结构体，用于管理发送的 TCP 数据段
    struct Segment {
        TCPPacket packet;
        std::chrono::steady_clock::time_point send_time;
        std::chrono::milliseconds timeout;
        int retries = 0;
    };

    // 存放重传段的列表
    std::list<Segment> segments_;

    // 重传回调函数
    std::function<void(const TCPPacket&)> retransmit_callback_;

    // 内部方法声明
    std::chrono::milliseconds calculate_rto() const;
    void update_rto(double rtt);
};

} // namespace lwip
