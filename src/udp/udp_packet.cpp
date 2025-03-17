#include "udp_packet.hpp"
#include <arpa/inet.h>
#include <cstring>

namespace lwip {

UDPPacket::UDPPacket() {
    memset(&header_, 0, sizeof(header_));
}

bool UDPPacket::parse(const std::vector<uint8_t>& raw_data) {
    if (raw_data.size() < sizeof(UDPHeader)) {
        return false;
    }
    
    memcpy(&header_, raw_data.data(), sizeof(UDPHeader));
    header_.source_port = ntohs(header_.source_port);
    header_.dest_port = ntohs(header_.dest_port);
    header_.length = ntohs(header_.length);
    
    payload_.assign(raw_data.begin() + sizeof(UDPHeader), raw_data.end());
    return true;
}

std::vector<uint8_t> UDPPacket::serialize() const {
    std::vector<uint8_t> result;
    result.reserve(sizeof(UDPHeader) + payload_.size());
    
    UDPHeader net_header = header_;
    net_header.source_port = htons(header_.source_port);
    net_header.dest_port = htons(header_.dest_port);
    net_header.length = htons(sizeof(UDPHeader) + payload_.size());
    net_header.checksum = calculate_checksum();
    
    result.insert(result.end(), 
                 reinterpret_cast<uint8_t*>(&net_header),
                 reinterpret_cast<uint8_t*>(&net_header) + sizeof(UDPHeader));
    result.insert(result.end(), payload_.begin(), payload_.end());
    
    return result;
}

uint16_t UDPPacket::calculate_checksum() const {
    uint32_t sum = 0;
    
    // UDP伪头部
    sum += (header_.source_port << 16) | header_.dest_port;
    sum += header_.length;
    
    // 数据部分
    const uint16_t* data = reinterpret_cast<const uint16_t*>(payload_.data());
    for (size_t i = 0; i < payload_.size() / 2; ++i) {
        sum += ntohs(data[i]);
    }
    
    // 如果数据长度为奇数，处理最后一个字节
    if (payload_.size() % 2) {
        sum += payload_.back() << 8;
    }
    
    // 折叠32位和
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

} // namespace lwip
