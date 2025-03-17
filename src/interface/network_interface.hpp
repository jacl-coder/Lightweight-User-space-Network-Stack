#pragma once
#include <cstdint>
#include <vector>
#include <memory>

namespace lwip {

class NetworkInterface {
public:
    virtual ~NetworkInterface() = default;
    
    virtual bool init() = 0;
    virtual bool send_frame(const std::vector<uint8_t>& data) = 0;
    virtual bool receive_frame(std::vector<uint8_t>& data) = 0;
    
    virtual uint64_t get_mac_address() const = 0;
    virtual void set_mac_address(uint64_t mac) = 0;
};

} // namespace lwip
