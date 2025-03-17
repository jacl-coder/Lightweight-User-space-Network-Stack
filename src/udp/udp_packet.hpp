#pragma once
#include <cstdint>
#include <vector>

namespace lwip {

class UDPPacket {
public:
    struct UDPHeader {
        uint16_t source_port;
        uint16_t dest_port;
        uint16_t length;
        uint16_t checksum;
    };

    UDPPacket();
    bool parse(const std::vector<uint8_t>& raw_data);
    std::vector<uint8_t> serialize() const;
    uint16_t calculate_checksum() const;

    // 添加访问方法
    uint16_t get_source_port() const { return header_.source_port; }
    uint16_t get_dest_port() const { return header_.dest_port; }
    const std::vector<uint8_t>& get_payload() const { return payload_; }
    void set_payload(const std::vector<uint8_t>& data) { payload_ = data; }

private:
    UDPHeader header_;
    std::vector<uint8_t> payload_;
};

} // namespace lwip
