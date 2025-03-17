#pragma once
#include <cstdint>
#include <vector>
#include <cstring>
#include <arpa/inet.h>

namespace lwip {

class IPPacket {
public:
    struct IPHeader {
        uint8_t version_ihl;
        uint8_t type_of_service;
        uint16_t total_length;
        uint16_t identification;
        uint16_t flags_fragment_offset;
        uint8_t time_to_live;
        uint8_t protocol;
        uint16_t header_checksum;
        uint32_t source_address;
        uint32_t destination_address;
    };

    IPPacket() {
        memset(&header_, 0, sizeof(header_));
        header_.version_ihl = 0x45;  // IPv4, header length 20 bytes
        header_.time_to_live = 64;
    }
    
    bool parse(const std::vector<uint8_t>& raw_data);
    std::vector<uint8_t> serialize() const;
    void set_payload(const std::vector<uint8_t>& data) { payload_ = data; }
    uint16_t calculate_header_checksum() const;
    void set_addresses(uint32_t src, uint32_t dst) {
        header_.source_address = src;
        header_.destination_address = dst;
    }

    // 分片相关方法
    void set_fragment_offset(uint16_t offset) {
        header_.flags_fragment_offset = (header_.flags_fragment_offset & 0xE000) | (offset & 0x1FFF);
    }

    void set_more_fragments(bool more) {
        if (more) {
            header_.flags_fragment_offset |= 0x2000;
        } else {
            header_.flags_fragment_offset &= ~0x2000;
        }
    }

    uint16_t get_fragment_offset() const {
        return header_.flags_fragment_offset & 0x1FFF;
    }

    const std::vector<uint8_t>& get_payload() const {
        return payload_;
    }
    
private:
    IPHeader header_;
    std::vector<uint8_t> payload_;
};

uint16_t IPPacket::calculate_header_checksum() const {
    uint32_t sum = 0;
    const uint16_t* data = reinterpret_cast<const uint16_t*>(&header_);
    
    for (size_t i = 0; i < sizeof(header_) / 2; ++i) {
        sum += ntohs(data[i]);
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

} // namespace lwip
