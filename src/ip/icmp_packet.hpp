#pragma once
#include <cstdint>
#include <vector>

namespace lwip {

class ICMPPacket {
public:
    struct ICMPHeader {
        uint8_t type;
        uint8_t code;
        uint16_t checksum;
        uint32_t rest_of_header;
    };

    ICMPPacket();
    bool create_echo_request(uint16_t identifier, uint16_t sequence);
    bool create_echo_reply(const ICMPPacket& request);
    uint8_t get_type() const { return header_.type; }
    uint16_t calculate_checksum() const;
    std::vector<uint8_t> serialize() const;

private:
    ICMPHeader header_;
    std::vector<uint8_t> payload_;

    uint16_t calculate_checksum() const {
        uint32_t sum = 0;
        const uint16_t* ptr = reinterpret_cast<const uint16_t*>(&header_);
        
        // 计算头部校验和
        for (size_t i = 0; i < sizeof(header_) / 2; ++i) {
            sum += ntohs(ptr[i]);
        }
        
        // 计算负载校验和
        ptr = reinterpret_cast<const uint16_t*>(payload_.data());
        for (size_t i = 0; i < payload_.size() / 2; ++i) {
            sum += ntohs(ptr[i]);
        }
        
        // 如果负载长度为奇数，处理最后一个字节
        if (payload_.size() % 2) {
            sum += (payload_.back() << 8);
        }
        
        // 折叠32位和到16位
        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
        
        return ~sum;
    }
};

bool ICMPPacket::create_echo_reply(const ICMPPacket& request) {
    header_.type = 0;  // ECHO响应
    header_.code = 0;
    header_.rest_of_header = request.header_.rest_of_header;
    payload_ = request.payload_;
    header_.checksum = calculate_checksum();
    return true;
}

} // namespace lwip
