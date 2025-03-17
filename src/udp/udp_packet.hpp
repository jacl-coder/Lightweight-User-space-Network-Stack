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

private:
    UDPHeader header_;
    std::vector<uint8_t> payload_;
};

} // namespace lwip
