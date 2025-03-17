#pragma once
#include <cstdint>
#include <vector>

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
    bool parse(const std::vector<uint8_t>& raw_data);
    std::vector<uint8_t> serialize() const;
    uint16_t calculate_checksum() const;

private:
    TCPHeader header_;
    std::vector<uint8_t> payload_;
};

} // namespace lwip
