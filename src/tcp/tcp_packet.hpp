#pragma once
#include <cstdint>
#include <vector>
#include <cstring>

namespace lwip {

class TCPPacket {
public:
    struct TCPHeader {
        uint16_t source_port;
        uint16_t dest_port;
        uint32_t sequence_number;
        uint32_t ack_number;
        uint16_t flags;
        uint16_t window_size;
        uint16_t checksum;
        uint16_t urgent_pointer;
    };

    TCPPacket();
    
    // TCP选项设置方法
    void set_mss(uint16_t mss) { options_.mss = mss; }
    void set_window_scale(uint8_t scale) { options_.window_scale = scale; }
    void enable_timestamps(bool enable) { options_.timestamps_enabled = enable; }
    
    // 数据访问方法
    void set_payload(const std::vector<uint8_t>& data) { payload_ = data; }
    const std::vector<uint8_t>& get_payload() const { return payload_; }
    size_t get_payload_size() const { return payload_.size(); }

    bool parse(const std::vector<uint8_t>& raw_data);
    std::vector<uint8_t> serialize() const;
    bool verify_checksum() const;
    uint16_t calculate_checksum() const;

    // 头部字段访问方法
    void set_source_port(uint16_t port) { header_.source_port = port; }
    void set_dest_port(uint16_t port) { header_.dest_port = port; }
    void set_sequence_number(uint32_t seq) { header_.sequence_number = seq; }
    void set_ack_number(uint32_t ack) { header_.ack_number = ack; }
    void set_flags(uint16_t flags) { header_.flags = flags; }
    void set_window_size(uint16_t size) { header_.window_size = size; }
    
    uint16_t get_source_port() const { return header_.source_port; }
    uint16_t get_dest_port() const { return header_.dest_port; }
    uint32_t get_sequence_number() const { return header_.sequence_number; }
    uint32_t get_ack_number() const { return header_.ack_number; }
    uint16_t get_flags() const { return header_.flags; }
    uint16_t get_window_size() const { return header_.window_size; }

private:
    TCPHeader header_;
    std::vector<uint8_t> payload_;
    
    // TCP选项
    struct {
        uint16_t mss{1460};
        uint8_t window_scale{0};
        bool timestamps_enabled{false};
    } options_;
};

// TCP标志位定义
constexpr uint16_t TCP_FLAG_FIN = 0x0001;
constexpr uint16_t TCP_FLAG_SYN = 0x0002;
constexpr uint16_t TCP_FLAG_RST = 0x0004;
constexpr uint16_t TCP_FLAG_PSH = 0x0008;
constexpr uint16_t TCP_FLAG_ACK = 0x0010;
constexpr uint16_t TCP_FLAG_URG = 0x0020;

} // namespace lwip
