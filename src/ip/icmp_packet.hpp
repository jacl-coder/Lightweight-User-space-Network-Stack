#pragma once
#include <cstdint>
#include <vector>
#include <cstring>

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
    bool parse(const std::vector<uint8_t>& data);
    std::vector<uint8_t> serialize() const;
    uint16_t calculate_checksum() const;
    uint8_t get_type() const { return header_.type; }

private:
    ICMPHeader header_;
    std::vector<uint8_t> payload_;
};

} // namespace lwip
