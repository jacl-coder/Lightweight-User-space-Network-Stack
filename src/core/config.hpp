#pragma once
#include <cstdint>
#include <string>
#include "../utils/inet_address.hpp"

namespace lwip {

struct StackConfig {
    uint16_t mtu{1500};
    uint32_t tcp_timeout_ms{30000};
    uint32_t buffer_pool_size{1024};
    uint16_t max_connections{1000};
    InetAddress local_address;
};

class Config {
public:
    static Config& instance();
    
    void load_from_file(const std::string& filename);
    const StackConfig& get_config() const { return config_; }
    
private:
    Config() = default;
    StackConfig config_;
};

} // namespace lwip
