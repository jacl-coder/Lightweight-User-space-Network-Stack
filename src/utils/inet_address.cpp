#include "inet_address.hpp"
#include <arpa/inet.h>
#include <cstring>

namespace lwip {

InetAddress::InetAddress(const std::string& ip, uint16_t port) : port_(port) {
    ip_ = inet_addr(ip.c_str());
}

InetAddress::InetAddress(uint32_t ip, uint16_t port) 
    : ip_(ip), port_(port) {
}

std::string InetAddress::to_string() const {
    struct in_addr addr;
    addr.s_addr = htonl(ip_);
    return inet_ntoa(addr);
}

} // namespace lwip
