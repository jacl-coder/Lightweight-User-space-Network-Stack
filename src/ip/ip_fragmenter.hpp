#pragma once
#include "ip_packet.hpp"
#include <map>
#include <chrono>

#define MAX_FRAGMENTS 1000

namespace lwip {

class IPFragmenter {
public:
    std::vector<IPPacket> fragment_packet(const IPPacket& packet, uint16_t mtu);
    bool add_fragment(const IPPacket& fragment);
    bool is_complete() const;
    IPPacket reassemble();

    // 添加缺失的方法声明
    void cleanup_timeout_fragments();
    void cleanup();
    void discover_path_mtu(uint32_t destination_ip);
    bool try_mtu_size(uint32_t destination_ip, uint16_t mtu_size);

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
