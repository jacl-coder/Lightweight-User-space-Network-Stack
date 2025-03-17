#pragma once
#include <array>
#include <vector>
#include <cstdint>
#include <cstring>

namespace lwip {

class IPv6Packet {
public:
    struct IPv6Header {
        uint32_t version_class_flow;  // 版本(4位),流量类别(8位),流标签(20位)
        uint16_t payload_length;      // 有效载荷长度
        uint8_t  next_header;         // 下一个头部
        uint8_t  hop_limit;           // 跳数限制
        std::array<uint8_t, 16> source_address;      // 源地址
        std::array<uint8_t, 16> destination_address; // 目的地址
    };

    IPv6Packet();
    bool parse(const std::vector<uint8_t>& raw_data);
    std::vector<uint8_t> serialize() const;
    void set_payload(const std::vector<uint8_t>& payload);

private:
    IPv6Header header_;
    std::vector<uint8_t> payload_;
};

} // namespace lwip
