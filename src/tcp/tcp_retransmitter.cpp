#include "tcp_retransmitter.hpp"
#include "../utils/logger.hpp"

namespace lwip {

void TCPRetransmitter::add_segment(const TCPPacket& packet) {
    // 使用 push_back 替代 push
    segments_.push_back({
        packet,
        std::chrono::steady_clock::now(),
        calculate_rto(),
        0  // retries 默认值
    });
}

void TCPRetransmitter::handle_ack(uint32_t ack_number) {
    auto now = std::chrono::steady_clock::now();
    
    // 移除已确认的段
    segments_.remove_if([ack_number](const Segment& seg) {
        return seg.packet.get_sequence_number() + seg.packet.get_payload_size() <= ack_number;
    });

    // 更新RTO
    if (!segments_.empty()) {
        auto& first = segments_.front();
        double rtt = std::chrono::duration<double>(now - first.send_time).count() * 1000;
        update_rto(rtt);
    }
}

void TCPRetransmitter::check_timeouts() {
    auto now = std::chrono::steady_clock::now();
    
    for (auto& segment : segments_) {
        if (now - segment.send_time > segment.timeout) {
            // 重传
            segment.retries++;
            segment.timeout *= 2;  // 指数退避
            segment.send_time = now;
            
            // 触发重传回调
            if (retransmit_callback_) {
                retransmit_callback_(segment.packet);
            }
        }
    }
}

std::chrono::milliseconds TCPRetransmitter::calculate_rto() const {
    return std::chrono::milliseconds(static_cast<int>(rto_));
}

void TCPRetransmitter::update_rto(double rtt) {
    if (srtt_ == 0) {
        srtt_ = rtt;
        rttvar_ = rtt / 2;
    } else {
        rttvar_ = 0.75 * rttvar_ + 0.25 * std::abs(srtt_ - rtt);
        srtt_ = 0.875 * srtt_ + 0.125 * rtt;
    }
    
    rto_ = srtt_ + 4 * rttvar_;
    rto_ = std::max(rto_, static_cast<double>(RTO_MIN.count()));
    rto_ = std::min(rto_, static_cast<double>(RTO_MAX.count()));
}

} // namespace lwip
