#pragma once
#include <string>
#include <future>
#include <vector>
#include "../utils/inet_address.hpp"  // 改用 utils 目录下的 InetAddress
#include "../udp/udp_packet.hpp"

namespace lwip {

class DNSResolver {
public:
    static DNSResolver& instance();
    
    std::future<InetAddress> resolve(const std::string& hostname);
    void add_dns_server(const InetAddress& server);
    void set_timeout(std::chrono::milliseconds timeout);
    void handle_query(const UDPPacket& packet);

private:
    struct DNSHeader {
        uint16_t id;
        uint16_t flags;
        uint16_t questions;
        uint16_t answers;
        uint16_t authority;
        uint16_t additional;
    };
    
    std::vector<InetAddress> dns_servers_;
    std::chrono::milliseconds timeout_{5000};
};

} // namespace lwip
