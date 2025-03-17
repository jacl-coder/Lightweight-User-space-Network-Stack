#include "tcp_connection.hpp"
#include "../core/stack_manager.hpp"
#include "../utils/logger.hpp"

namespace lwip {

TCPConnection::TCPConnection()
    : state_(TCPState::CLOSED)
    , local_seq_num_(rand())
    , remote_seq_num_(0)
    , local_port_(0)
    , remote_port_(0)
    , remote_ip_(0) {
}

bool TCPConnection::connect(uint32_t remote_ip, uint16_t remote_port) {
    if (state_ != TCPState::CLOSED) {
        return false;
    }

    remote_ip_ = remote_ip;
    remote_port_ = remote_port;
    
    TCPPacket syn_packet;
    syn_packet.header_.source_port = local_port_;
    syn_packet.header_.dest_port = remote_port;
    syn_packet.header_.sequence_number = local_seq_num_;
    syn_packet.header_.flags = TCP_FLAG_SYN;
    syn_packet.header_.window_size = 65535;
    
    auto& stack = StackManager::instance();
    if (!stack.send_packet(syn_packet.serialize())) {
        return false;
    }
    
    state_ = TCPState::SYN_SENT;
    return true;
}

bool TCPConnection::send(const std::vector<uint8_t>& data) {
    if (state_ != TCPState::ESTABLISHED) {
        return false;
    }

    // 检查发送窗口
    size_t available_window = std::min(window_.send_window_, window_.cwnd_);
    if (available_window < data.size()) {
        return false;
    }

    // 分片发送
    constexpr size_t MSS = 1460;
    size_t offset = 0;
    
    while (offset < data.size()) {
        size_t chunk_size = std::min(MSS, data.size() - offset);
        TCPPacket packet;
        
        packet.header_.sequence_number = local_seq_num_;
        packet.header_.flags = TCP_FLAG_ACK;
        packet.set_payload({data.begin() + offset, data.begin() + offset + chunk_size});
        
        if (!stack_manager_.send_packet(packet)) {
            return false;
        }

        // 添加到重传队列
        retransmit_queue_.push({
            packet.get_payload(),
            local_seq_num_, 
            std::chrono::steady_clock::now(),
            0
        });

        local_seq_num_ += chunk_size;
        offset += chunk_size;
    }

    return true;
}

bool TCPConnection::handle_packet(const TCPPacket& packet) {
    switch (state_) {
        case TCPState::CLOSED:
            if (packet.header_.flags & TCP_FLAG_SYN) {
                // 被动打开
                remote_seq_num_ = packet.header_.sequence_number;
                state_ = TCPState::SYN_RECEIVED;
                send_syn_ack();
            }
            break;
            
        case TCPState::SYN_SENT:
            if (packet.header_.flags & (TCP_FLAG_SYN | TCP_FLAG_ACK)) {
                remote_seq_num_ = packet.header_.sequence_number + 1;
                if (packet.header_.ack_number == local_seq_num_ + 1) {
                    local_seq_num_++;
                    state_ = TCPState::ESTABLISHED;
                    send_ack();
                }
            }
            break;
            
        case TCPState::ESTABLISHED:
            if (packet.header_.flags & TCP_FLAG_FIN) {
                remote_seq_num_++;
                state_ = TCPState::CLOSE_WAIT;
                send_ack();
            } else if (packet.header_.flags & TCP_FLAG_ACK) {
                handle_data(packet);
                update_window(packet);
            }
            break;
            
        case TCPState::FIN_WAIT_1:
            if (packet.header_.flags & TCP_FLAG_ACK) {
                state_ = TCPState::FIN_WAIT_2;
            }
            break;
            
        // ...其他状态处理
    }
    return true;
}

void TCPConnection::handle_data(const TCPPacket& packet) {
    if (packet.get_payload_size() > 0) {
        // 检查序列号
        if (packet.header_.sequence_number == remote_seq_num_) {
            process_in_order_data(packet);
        } else {
            store_out_of_order_data(packet);
        }
    }
}

void TCPConnection::update_window(const TCPPacket& packet) {
    // 更新发送窗口
    window_.send_window_ = packet.header_.window_size;
    
    // 更新拥塞窗口
    if (packet.header_.flags & TCP_FLAG_ACK) {
        update_congestion_control(false);
    }
}

void TCPConnection::close() {
    if (state_ == TCPState::ESTABLISHED) {
        // 发送FIN包
        state_ = TCPState::FIN_WAIT_1;
    }
}

void TCPConnection::update_congestion_control(bool is_timeout) {
    if (is_timeout) {
        // 超时,进入慢启动
        window_.ssthresh_ = window_.cwnd_ / 2;
        window_.cwnd_ = MSS;
        congestion_state_ = CongestionState::SLOW_START;
    } else {
        switch (congestion_state_) {
            case CongestionState::SLOW_START:
                if (window_.cwnd_ >= window_.ssthresh_) {
                    congestion_state_ = CongestionState::CONGESTION_AVOIDANCE;
                } else {
                    window_.cwnd_ += MSS;
                }
                break;
                
            case CongestionState::CONGESTION_AVOIDANCE:
                window_.cwnd_ += MSS * MSS / window_.cwnd_;
                break;
                
            case CongestionState::FAST_RECOVERY:
                window_.cwnd_ = window_.ssthresh_;
                congestion_state_ = CongestionState::CONGESTION_AVOIDANCE;
                break;
        }
    }
}

class TCPConnection {
// ...existing code...

private:
    // 添加滑动窗口相关成员
    struct WindowControl {
        uint32_t send_window_{65535};
        uint32_t recv_window_{65535};
        uint32_t cwnd_{1460};        // 拥塞窗口
        uint32_t ssthresh_{65535};   // 慢启动阈值
    } window_;

    // 添加拥塞控制状态
    enum class CongestionState {
        SLOW_START,
        CONGESTION_AVOIDANCE,
        FAST_RECOVERY
    } congestion_state_{CongestionState::SLOW_START};
    
    // 重传队列
    struct RetransmitSegment {
        std::vector<uint8_t> data;
        uint32_t sequence_number;
        std::chrono::steady_clock::time_point sent_time;
        int retries{0};
    };
    std::queue<RetransmitSegment> retransmit_queue_;
};

} // namespace lwip
