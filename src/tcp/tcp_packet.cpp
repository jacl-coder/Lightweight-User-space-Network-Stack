#include "tcp_packet.hpp"
#include <arpa/inet.h>

namespace lwip {

TCPPacket::TCPPacket() {
    memset(&header_, 0, sizeof(header_));
}

bool TCPPacket::parse(const std::vector<uint8_t>& raw_data) {
    if (raw_data.size() < sizeof(TCPHeader)) {
        return false;
    }
    
    // 解析基本头部
    memcpy(&header_, raw_data.data(), sizeof(TCPHeader));
    
    // 转换网络字节序
    header_.source_port = ntohs(header_.source_port);
    header_.dest_port = ntohs(header_.dest_port);
    header_.sequence_number = ntohl(header_.sequence_number);
    header_.ack_number = ntohl(header_.ack_number);
    header_.window_size = ntohs(header_.window_size);
    
    // 解析选项
    size_t header_len = (header_.flags >> 12) * 4;
    if (raw_data.size() >= header_len) {
        [[maybe_unused]] const size_t options_offset = sizeof(TCPHeader);
        [[maybe_unused]] const size_t options_len = header_len - sizeof(TCPHeader);
        
        // 解析选项...
        // 这里可以添加选项解析代码
        
        // 设置负载
        payload_.assign(raw_data.begin() + header_len, raw_data.end());
    }
    
    return true;
}

std::vector<uint8_t> TCPPacket::serialize() const {
    std::vector<uint8_t> result;
    result.reserve(sizeof(TCPHeader) + payload_.size());
    
    // 计算选项长度
    size_t options_len = 0;
    if (options_.mss != 0) options_len += 4;
    if (options_.window_scale != 0) options_len += 3;
    if (options_.timestamps_enabled) options_len += 10;
    
    // 确保选项长度是4的倍数
    size_t padding = (4 - (options_len % 4)) % 4;
    options_len += padding;
    
    // 设置头部长度（4字节对齐）
    size_t header_len = sizeof(TCPHeader) + options_len;
    const_cast<TCPPacket*>(this)->header_.flags &= 0x0FFF;
    const_cast<TCPPacket*>(this)->header_.flags |= ((header_len / 4) << 12);
    
    TCPHeader net_header = header_;
    net_header.source_port = htons(header_.source_port);
    net_header.dest_port = htons(header_.dest_port);
    net_header.sequence_number = htonl(header_.sequence_number);
    net_header.ack_number = htonl(header_.ack_number);
    net_header.window_size = htons(header_.window_size);
    net_header.checksum = calculate_checksum();
    
    result.insert(result.end(),
                 reinterpret_cast<const uint8_t*>(&net_header),
                 reinterpret_cast<const uint8_t*>(&net_header) + sizeof(TCPHeader));
    result.insert(result.end(), payload_.begin(), payload_.end());
    
    return result;
}

bool TCPPacket::verify_checksum() const {
    uint16_t saved_checksum = header_.checksum;
    const_cast<TCPPacket*>(this)->header_.checksum = 0;
    
    uint16_t calculated = calculate_checksum();
    const_cast<TCPPacket*>(this)->header_.checksum = saved_checksum;
    
    return saved_checksum == calculated;
}

uint16_t TCPPacket::calculate_checksum() const {
    uint32_t sum = 0;
    
    // 计算伪首部的校验和
    [[maybe_unused]] struct PseudoHeader {
        uint32_t source_ip;
        uint32_t dest_ip;
        uint8_t  zeros;
        uint8_t  protocol;
        uint16_t tcp_length;
    } pseudo;
    
    pseudo.protocol = 6;  // TCP protocol number
    pseudo.tcp_length = htons(sizeof(TCPHeader) + payload_.size());
    
    // 计算TCP头部和数据的校验和
    const uint16_t* data = reinterpret_cast<const uint16_t*>(&header_);
    for (size_t i = 0; i < sizeof(TCPHeader) / 2; ++i) {
        sum += ntohs(data[i]);
    }
    
    // 计算数据部分的校验和
    for (size_t i = 0; i < payload_.size() / 2; ++i) {
        sum += (payload_[i * 2] << 8) | payload_[i * 2 + 1];
    }
    
    // 处理奇数长度的数据
    if (payload_.size() & 1) {
        sum += payload_.back() << 8;
    }
    
    // 进行校验和的折叠
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

} // namespace lwip
