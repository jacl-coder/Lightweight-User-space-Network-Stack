#pragma once
#include <string>
#include <netinet/in.h>

namespace lwip {

class InetAddress {
public:
    InetAddress(const std::string &address, unsigned short port);
    InetAddress(unsigned int address, unsigned short port);
    
    in_addr get_addr() const { return addr_; }
    unsigned short get_port() const { return port_; }
    
private:
    in_addr addr_;
    unsigned short port_;
};

} // namespace lwip
