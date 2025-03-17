#pragma once
#include "ip_packet.hpp"
#include <map>
#include <chrono>

namespace lwip {

class IPFragmenter {
public:
    std::vector<IPPacket> fragment_packet(const IPPacket& packet, uint16_t mtu);
    bool add_fragment(const IPPacket& fragment);
    bool is_complete() const;
    IPPacket reassemble();

private:
    struct FragmentInfo {
        std::vector<uint8_t> data;
        bool received{false};
        std::chrono::steady_clock::time_point timestamp;
    };
    
    std::map<uint16_t, FragmentInfo> fragments_;
    uint16_t total_length_{0};
};

} // namespace lwip
