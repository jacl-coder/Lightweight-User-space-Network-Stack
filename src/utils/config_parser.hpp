#pragma once
#include <string>
#include <map>
#include <cstdint>

namespace lwip {

struct StackConfig {
    std::string interface_name{"tun0"};
    std::string ip_address{"192.168.1.100"};
    std::string netmask{"255.255.255.0"};
    uint16_t mtu{1500};
};

class ConfigParser {
public:
    bool parse_file(const std::string& filename, StackConfig& config);

private:
    bool apply_config(const std::map<std::string, std::string>& values, StackConfig& config);
};

} // namespace lwip
