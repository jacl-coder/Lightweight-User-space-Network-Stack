#include "tcp_connection.hpp"
#include "../core/stack_manager.hpp"
#include "../utils/logger.hpp"
#include <algorithm>

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
    syn_packet.set_source_port(local_port_);
    syn_packet.set_dest_port(remote_port);
    syn_packet.set_sequence_number(local_seq_num_);
    syn_packet.set_flags(TCP_FLAG_SYN);
    syn_packet.set_window_size(65535);
    
    StackManager& stack = StackManager::instance();  // 使用引用
    if (!stack.send_packet(syn_packet.serialize())) {
        LOG_ERROR("Failed to send SYN packet");
        return false;
    }
    
    state_ = TCPState::SYN_SENT;
    return true;
}

bool TCPConnection::send(const std::vector<uint8_t>& data) {
    if (state_ != TCPState::ESTABLISHED) {
        return false;
    }

    // 实现Nagle算法
    if (nagle_enabled_ && unacked_data() > 0 && data.size() < max_segment_size_) {
        pending_data_.insert(pending_data_.end(), data.begin(), data.end());
        return true;
    }

    // 考虑发送窗口和拥塞窗口
    size_t window = std::min(
        static_cast<size_t>(tcp_params_.send_window),
        static_cast<size_t>(tcp_params_.cwnd * max_segment_size_)
    );
    
    // 分片发送
    size_t offset = 0;
    while (offset < data.size()) {
        size_t chunk_size = std::min(
            std::min(max_segment_size_, static_cast<uint16_t>(data.size() - offset)),
            static_cast<uint16_t>(window)
        );
        
        TCPPacket packet;
        
        packet.set_sequence_number(local_seq_num_);
        packet.set_flags(TCP_FLAG_ACK);
        packet.set_payload({data.begin() + offset, data.begin() + offset + chunk_size});
        
        StackManager& stack = StackManager::instance();  // 使用引用
        if (!stack.send_packet(packet.serialize())) {
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
    last_activity_ = std::chrono::steady_clock::now();

    switch (state_) {
        case TCPState::CLOSED:
            if (packet.get_flags() & TCP_FLAG_SYN) {
                // 收到SYN，准备接受新连接
                remote_seq_num_ = packet.get_sequence_number();
                send_syn_ack();
                state_ = TCPState::SYN_RECEIVED;
            }
            break;
            
        case TCPState::LISTEN:
            if (packet.get_flags() & TCP_FLAG_SYN) {
                remote_seq_num_ = packet.get_sequence_number();
                send_syn_ack();
                state_ = TCPState::SYN_RECEIVED;
            }
            break;
            
        case TCPState::SYN_SENT:
            if (packet.get_flags() & (TCP_FLAG_SYN | TCP_FLAG_ACK)) {
                remote_seq_num_ = packet.get_sequence_number() + 1;
                if (packet.get_ack_number() == local_seq_num_ + 1) {
                    local_seq_num_++;
                    send_ack();
                    state_ = TCPState::ESTABLISHED;
                }
            }
            break;
            
        case TCPState::SYN_RECEIVED:
            if (packet.get_flags() & TCP_FLAG_ACK) {
                state_ = TCPState::ESTABLISHED;
            }
            break;
            
        case TCPState::ESTABLISHED:
            if (packet.get_flags() & TCP_FLAG_FIN) {
                state_ = TCPState::CLOSE_WAIT;
                send_ack();
            } else if (packet.get_flags() & TCP_FLAG_ACK) {
                handle_data(packet);
                update_window(packet);
            }
            break;
            
        case TCPState::FIN_WAIT_1:
            if (packet.get_flags() & TCP_FLAG_ACK) {
                state_ = TCPState::FIN_WAIT_2;
            }
            break;
            
        case TCPState::FIN_WAIT_2:
            if (packet.get_flags() & TCP_FLAG_FIN) {
                send_ack();
                state_ = TCPState::TIME_WAIT;
            }
            break;
            
        case TCPState::CLOSE_WAIT:
            // 等待应用层关闭连接
            break;
            
        case TCPState::CLOSING:
            if (packet.get_flags() & TCP_FLAG_ACK) {
                state_ = TCPState::TIME_WAIT;
            }
            break;
            
        case TCPState::LAST_ACK:
            if (packet.get_flags() & TCP_FLAG_ACK) {
                state_ = TCPState::CLOSED;
            }
            break;
            
        case TCPState::TIME_WAIT:
            // 实现2MSL等待
            break;
    }
    return true;
}

void TCPConnection::handle_data(const TCPPacket& packet) {
    if (packet.get_payload_size() > 0) {
        // 使用getter方法检查序列号
        if (packet.get_sequence_number() == remote_seq_num_) {
            process_in_order_data(packet);
        } else {
            store_out_of_order_data(packet);
        }
    }
}

void TCPConnection::update_window(const TCPPacket& packet) {
    // 更新发送窗口
    window_.send_window_ = packet.get_window_size();
    
    // 更新拥塞窗口
    if (packet.get_flags() & TCP_FLAG_ACK) {
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

void TCPConnection::update_rtt(std::chrono::milliseconds measured_rtt) {
    double r = measured_rtt.count();
    if (srtt_ == 0) {
        srtt_ = r;
        rttvar_ = r/2;
    } else {
        rttvar_ = 0.75 * rttvar_ + 0.25 * std::abs(srtt_ - r);
        srtt_ = 0.875 * srtt_ + 0.125 * r;
    }
    rto_ = std::chrono::milliseconds(
        static_cast<int>(srtt_ + std::max(1.0, 4*rttvar_))
    );
}

void TCPConnection::handle_timeout() {
    // 超时重传处理
    tcp_params_.ssthresh = std::max(tcp_params_.cwnd / 2, static_cast<uint32_t>(2));
    tcp_params_.cwnd = MSS;
    in_fast_recovery_ = false;
    
    if (state_ == TCPState::ESTABLISHED) {
        retransmit_unacked_segments();
    }
}

bool TCPConnection::is_timeout() const {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - last_activity_
    ).count() > TIMEOUT_SECONDS;
}

size_t TCPConnection::unacked_data() const {
    return unacked_segments_.size();
}

void TCPConnection::send_syn_ack() {
    TCPPacket packet;
    packet.set_flags(TCP_FLAG_SYN | TCP_FLAG_ACK);
    packet.set_sequence_number(local_seq_num_);
    packet.set_ack_number(remote_seq_num_ + 1);
    send_packet(packet);
}

void TCPConnection::send_ack() {
    TCPPacket packet;
    packet.set_flags(TCP_FLAG_ACK);
    packet.set_sequence_number(local_seq_num_);
    packet.set_ack_number(remote_seq_num_ + 1);
    send_packet(packet);
}

void TCPConnection::process_in_order_data(const TCPPacket& packet) {
    // 处理按序到达的数据
    received_data_.insert(received_data_.end(), 
                         packet.get_payload().begin(), 
                         packet.get_payload().end());
    remote_seq_num_ += packet.get_payload_size();
}

void TCPConnection::store_out_of_order_data(const TCPPacket& packet) {
    // 存储乱序数据
    out_of_order_data_[packet.get_sequence_number()] = packet.get_payload();
}

void TCPConnection::retransmit_unacked_segments() {
    // 重传未确认的数据段
    for (const auto& segment : unacked_segments_) {
        send_packet(segment);
    }
}

bool TCPConnection::send_packet(const TCPPacket& packet) {
    auto& stack = StackManager::instance();
    return stack.send_packet(packet.serialize());
}

bool TCPConnection::send_packet(const std::vector<uint8_t>& data) {
    auto& stack = StackManager::instance();
    return stack.send_packet(data);
}

bool TCPConnection::receive(std::vector<uint8_t>& data) {
    // 简单实现：将内部接收缓冲区返回，真实逻辑依据项目需求调整
    data = received_data_; // 假定 received_data_ 成员保存接收数据
    return true;
}

} // namespace lwip
