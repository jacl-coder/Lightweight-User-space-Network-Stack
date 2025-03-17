#pragma once
#include <cstdint>
#include <chrono>
#include <memory>
#include <queue>
#include <vector>
#include <map>
#include "tcp_packet.hpp"

namespace lwip {

// 前向声明
class StackManager;

enum class TCPState {
    CLOSED,
    LISTEN,
    SYN_SENT,
    SYN_RECEIVED,
    ESTABLISHED,
    FIN_WAIT_1,
    FIN_WAIT_2,
    CLOSE_WAIT,
    CLOSING,
    LAST_ACK,
    TIME_WAIT
};

enum class CongestionState {
    SLOW_START,
    CONGESTION_AVOIDANCE,
    FAST_RECOVERY
};

class TCPConnection {
public:
    static constexpr uint16_t MSS = 1460;  // 最大报文段长度
    static constexpr int TIMEOUT_SECONDS = 300;  // 添加超时常量

    TCPConnection();
    TCPConnection(uint32_t remote_ip, uint16_t remote_port);  // 添加构造函数
    
    // 连接管理
    bool connect(uint32_t remote_ip, uint16_t remote_port);
    bool accept();
    void close();
    bool is_connected() const { return state_ == TCPState::ESTABLISHED; }
    
    // 数据传输
    bool send(const std::vector<uint8_t>& data);
    bool receive(std::vector<uint8_t>& data);
    bool handle_packet(const TCPPacket& packet);
    bool send_data(const std::vector<uint8_t>& data);
    
    // TCP选项配置
    void set_window_size(uint16_t size) { send_window_size_ = size; }
    void set_mss(uint16_t mss) { max_segment_size_ = mss; }
    void enable_timestamps(bool enable) { timestamps_enabled_ = enable; }
    void enable_nagle(bool enable) { nagle_enabled_ = enable; }
    void set_keep_alive(bool enable, uint32_t idle_time = 7200);
    
    // 端口管理
    void set_local_port(uint16_t port) { local_port_ = port; }
    uint16_t get_local_port() const { return local_port_; }
    
    // 拥塞控制
    void handle_timeout();
    bool is_timeout() const;  // 添加超时检查方法

private:
    // 基本状态
    TCPState state_;
    uint32_t local_seq_num_;
    uint32_t remote_seq_num_;
    uint16_t local_port_;
    uint16_t remote_port_;
    uint32_t remote_ip_;
    std::chrono::steady_clock::time_point last_activity_;
    static constexpr auto TIMEOUT = std::chrono::minutes(5);
    
    // 配置参数
    uint16_t max_segment_size_{MSS};
    uint16_t send_window_size_{65535};
    bool nagle_enabled_{true};
    bool timestamps_enabled_{false};
    
    // 拥塞控制
    struct WindowControl {
        uint32_t send_window_{65535};
        uint32_t recv_window_{65535};
        uint32_t cwnd_{MSS};        // 拥塞窗口
        uint32_t ssthresh_{65535};  // 慢启动阈值
    } window_;
    
    CongestionState congestion_state_{CongestionState::SLOW_START};
    
    // 重传控制
    struct RetransmitSegment {
        std::vector<uint8_t> data;
        uint32_t sequence_number;
        std::chrono::steady_clock::time_point sent_time;
        int retries{0};
    };
    std::queue<RetransmitSegment> retransmit_queue_;
    std::chrono::milliseconds rto_{1000};  // 重传超时
    double srtt_{0};    // 平滑往返时间
    double rttvar_{0};  // RTT变化

    // 发送缓冲区
    std::vector<uint8_t> pending_data_;
    bool in_fast_recovery_{false};
    
    // 管理器引用
    std::weak_ptr<StackManager> stack_manager_;
    
    // TCP连接参数
    struct {
        uint32_t cwnd{MSS};          // 拥塞窗口
        uint32_t ssthresh{65535};    // 慢启动阈值
        uint32_t send_window{65535}; // 发送窗口
        uint32_t recv_window{65535}; // 接收窗口
    } tcp_params_;

    // 数据缓冲
    std::vector<uint8_t> received_data_;
    std::map<uint32_t, std::vector<uint8_t>> out_of_order_data_;
    std::vector<TCPPacket> unacked_segments_;
    
    // 辅助方法
    bool send_packet(const TCPPacket& packet);
    bool send_packet(const std::vector<uint8_t>& data);

    // 内部方法
    void handle_data(const TCPPacket& packet);
    void update_window(const TCPPacket& packet);
    void update_rtt(std::chrono::milliseconds measured_rtt);
    void update_congestion_control(bool is_timeout);
    void process_in_order_data(const TCPPacket& packet);
    void store_out_of_order_data(const TCPPacket& packet);
    void retransmit_unacked_segments();
    size_t unacked_data() const;
    void send_syn_ack();
    void send_ack();
};

} // namespace lwip
