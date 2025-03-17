#include "icmp_packet.hpp"
#include <arpa/inet.h>

namespace lwip {

ICMPPacket::ICMPPacket() {
    memset(&header_, 0, sizeof(header_));
}

bool ICMPPacket::create_echo_request(uint16_t identifier, uint16_t sequence) {
    header_.type = 8;  // ECHO请求
    header_.code = 0;
    header_.rest_of_header = htonl((identifier << 16) | sequence);
    header_.checksum = calculate_checksum();
    return true;
}

bool ICMPPacket::parse(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(ICMPHeader)) {
        return false;
    }

    memcpy(&header_, data.data(), sizeof(ICMPHeader));
    payload_.assign(data.begin() + sizeof(ICMPHeader), data.end());

    // 验证校验和
    uint16_t received_checksum = header_.checksum;
    header_.checksum = 0;
    uint16_t calculated_checksum = calculate_checksum();
    header_.checksum = received_checksum;

    return received_checksum == calculated_checksum;
}

std::vector<uint8_t> ICMPPacket::serialize() const {
    std::vector<uint8_t> data;
    data.reserve(sizeof(ICMPHeader) + payload_.size());
    
    data.insert(data.end(), 
                reinterpret_cast<const uint8_t*>(&header_),
                reinterpret_cast<const uint8_t*>(&header_) + sizeof(ICMPHeader));
    data.insert(data.end(), payload_.begin(), payload_.end());
    
    return data;
}

uint16_t ICMPPacket::calculate_checksum() const {
    uint32_t sum = 0;
    const uint16_t* ptr = reinterpret_cast<const uint16_t*>(&header_);
    
    for (size_t i = 0; i < sizeof(header_) / 2; ++i) {
        sum += ntohs(ptr[i]);
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

} // namespace lwip
