#include "ipv6_nd.hpp"
#include "../utils/logger.hpp"
#include <array>

// 定义 NDMessage 结构（最低限度定义，用于编译）
struct NDMessage {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    std::array<uint8_t, 16> target_address;
};

namespace lwip {

bool IPv6NeighborDiscovery::send_neighbor_solicitation(const std::array<uint8_t, 16>& target_addr) {
    // 构造ICMPv6邻居请求消息
    struct NDMessage {
        uint8_t type{135};  // 邻居请求
        uint8_t code{0};
        uint16_t checksum{0};
        uint32_t reserved{0};
        std::array<uint8_t, 16> target_address;
    } message;

    message.target_address = target_addr;
    // TODO: 计算ICMPv6校验和
    
    return true;
}

void IPv6NeighborDiscovery::handle_neighbor_advertisement(const IPv6Packet& packet) {
    // 解析邻居通告消息
    auto& payload = packet.get_payload();
    if (payload.size() < sizeof(NDMessage)) {
        return;
    }

    const NDMessage* message = reinterpret_cast<const NDMessage*>(payload.data());
    if (message->type == 136) {  // 邻居通告
        NeighborEntry entry;
        // TODO: 从选项中提取MAC地址
        entry.timestamp = std::chrono::steady_clock::now();
        entry.is_reachable = true;
        neighbors_[message->target_address] = entry;
    }
}

bool IPv6NeighborDiscovery::lookup_neighbor(const std::array<uint8_t, 16>& ip, std::array<uint8_t, 6>& mac) {
    auto it = neighbors_.find(ip);
    if (it != neighbors_.end()) {
        auto now = std::chrono::steady_clock::now();
        if (now - it->second.timestamp <= NEIGHBOR_TIMEOUT) {
            mac = it->second.mac_address;
            return true;
        }
        neighbors_.erase(it);
    }
    return false;
}

} // namespace lwip
