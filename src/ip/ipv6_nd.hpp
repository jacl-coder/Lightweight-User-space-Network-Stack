#pragma once
#include <map>
#include <chrono>
#include <array>
#include "ipv6_packet.hpp"

namespace lwip {

class IPv6NeighborDiscovery {
public:
    struct NeighborEntry {
        std::array<uint8_t, 6> mac_address;
        std::chrono::steady_clock::time_point timestamp;
        bool is_reachable;
    };
    
    bool send_neighbor_solicitation(const std::array<uint8_t, 16>& target_addr);
    void handle_neighbor_advertisement(const IPv6Packet& packet);
    bool lookup_neighbor(const std::array<uint8_t, 16>& ip, std::array<uint8_t, 6>& mac);
    
private:
    static constexpr auto NEIGHBOR_TIMEOUT = std::chrono::minutes(20);
    std::map<std::array<uint8_t, 16>, NeighborEntry> neighbors_;
};

} // namespace lwip
