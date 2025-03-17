#include "ip_packet.hpp"
#include <arpa/inet.h>

namespace lwip {

bool IPPacket::parse(const std::vector<uint8_t>& raw_data) {
    if (raw_data.size() < sizeof(IPHeader)) {
        return false;
    }

    memcpy(&header_, raw_data.data(), sizeof(IPHeader));
    
    // 网络字节序转换
    header_.total_length = ntohs(header_.total_length);
    header_.identification = ntohs(header_.identification);
    header_.flags_fragment_offset = ntohs(header_.flags_fragment_offset);
    header_.header_checksum = ntohs(header_.header_checksum);
    
    size_t header_size = (header_.version_ihl & 0x0F) * 4;
    if (raw_data.size() >= header_size) {
        payload_.assign(raw_data.begin() + header_size, raw_data.end());
    }
    
    return true;
}

std::vector<uint8_t> IPPacket::serialize() const {
    std::vector<uint8_t> result;
    result.reserve(sizeof(IPHeader) + payload_.size());
    
    IPHeader net_header = header_;
    net_header.total_length = htons(sizeof(IPHeader) + payload_.size());
    net_header.identification = htons(header_.identification);
    net_header.flags_fragment_offset = htons(header_.flags_fragment_offset);
    net_header.header_checksum = calculate_header_checksum();
    
    result.insert(result.end(),
                 reinterpret_cast<const uint8_t*>(&net_header),
                 reinterpret_cast<const uint8_t*>(&net_header) + sizeof(IPHeader));
    result.insert(result.end(), payload_.begin(), payload_.end());
    
    return result;
}

} // namespace lwip
