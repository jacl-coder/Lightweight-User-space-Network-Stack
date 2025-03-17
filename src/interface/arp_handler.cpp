#include "arp_handler.hpp"
#include "../utils/logger.hpp"

namespace lwip {

bool ARPHandler::add_entry(const InetAddress& ip, uint64_t mac, bool is_static) {
    ARPEntry entry{mac, std::chrono::steady_clock::now(), is_static};
    arp_table_[ip.get_ip()] = entry;
    return true;
}

bool ARPHandler::resolve(const InetAddress& ip, uint64_t& mac) {
    auto it = arp_table_.find(ip.get_ip());
    if (it != arp_table_.end()) {
        if (!it->second.is_static) {
            auto now = std::chrono::steady_clock::now();
            if (now - it->second.timestamp > ENTRY_TIMEOUT) {
                arp_table_.erase(it);
                return false;
            }
        }
        mac = it->second.mac_address;
        return true;
    }
    return false;
}

void ARPHandler::cleanup_expired_entries() {
    auto now = std::chrono::steady_clock::now();
    for (auto it = arp_table_.begin(); it != arp_table_.end();) {
        if (!it->second.is_static && now - it->second.timestamp > ENTRY_TIMEOUT) {
            it = arp_table_.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace lwip
