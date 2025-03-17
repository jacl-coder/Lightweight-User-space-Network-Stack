#pragma once
#include <cstdint>
#include <string>
#include <array>

namespace lwip {

class InetAddress {
public:
    InetAddress() = default;
    explicit InetAddress(const std::string& ip, uint16_t port = 0);
    explicit InetAddress(uint32_t ip, uint16_t port = 0);

    std::string to_string() const;
    uint32_t get_ip() const { return ip_; }
    uint16_t get_port() const { return port_; }
    void set_port(uint16_t port) { port_ = port; }

private:
    uint32_t ip_{0};
    uint16_t port_{0};
};

} // namespace lwip
