#include "ipv6_packet.hpp"
#include <arpa/inet.h>
#include "../utils/logger.hpp"

namespace lwip {

IPv6Packet::IPv6Packet() {
    memset(&header_, 0, sizeof(header_));
    header_.version_class_flow = htonl(6 << 28);  // IPv6 version = 6
    header_.hop_limit = 64;
}

bool IPv6Packet::parse(const std::vector<uint8_t>& raw_data) {
    if (raw_data.size() < sizeof(IPv6Header)) {
        LOG_ERROR("IPv6 packet too small");
        return false;
    }
    
    memcpy(&header_, raw_data.data(), sizeof(IPv6Header));
    header_.payload_length = ntohs(header_.payload_length);
    
    payload_.assign(raw_data.begin() + sizeof(IPv6Header), raw_data.end());
    return true;
}

std::vector<uint8_t> IPv6Packet::serialize() const {
    std::vector<uint8_t> result;
    result.reserve(sizeof(IPv6Header) + payload_.size());
    
    IPv6Header net_header = header_;
    net_header.payload_length = htons(payload_.size());
    
    result.insert(result.end(), 
                 reinterpret_cast<const uint8_t*>(&net_header),
                 reinterpret_cast<const uint8_t*>(&net_header) + sizeof(IPv6Header));
    result.insert(result.end(), payload_.begin(), payload_.end());
    
    return result;
}

} // namespace lwip
