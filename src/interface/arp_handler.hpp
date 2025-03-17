#pragma once
#include <cstdint>
#include <map>
#include <chrono>
#include <vector>
#include "../utils/inet_address.hpp"

namespace lwip {

class ARPHandler {
public:
    struct ARPEntry {
        uint64_t mac_address;
        std::chrono::steady_clock::time_point timestamp;
        bool is_static;
    };

    bool add_entry(const InetAddress& ip, uint64_t mac, bool is_static = false);
    bool resolve(const InetAddress& ip, uint64_t& mac);
    void process_arp_packet(const std::vector<uint8_t>& packet);
    void cleanup_expired_entries();

private:
    static constexpr auto ENTRY_TIMEOUT = std::chrono::minutes(20);
    std::map<uint32_t, ARPEntry> arp_table_;
};

} // namespace lwip
