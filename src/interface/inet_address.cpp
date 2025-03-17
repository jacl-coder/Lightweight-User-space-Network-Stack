#include "inet_address.hpp"
#include <arpa/inet.h>

namespace lwip {

InetAddress::InetAddress(const std::string &address, unsigned short port) : port_(port) {
    inet_pton(AF_INET, address.c_str(), &addr_);
}

InetAddress::InetAddress(unsigned int address, unsigned short port) : port_(port) {
    addr_.s_addr = htonl(address);
}

} // namespace lwip
