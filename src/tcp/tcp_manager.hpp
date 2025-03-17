#pragma once
#include <memory>
#include <map>
#include <mutex>
#include "tcp_connection.hpp"
#include "../utils/inet_address.hpp"

namespace lwip {

class TCPManager {
public:
    TCPManager();
    
    std::shared_ptr<TCPConnection> create_connection();  // 添加声明
    std::shared_ptr<TCPConnection> find_or_create_connection(
        uint32_t remote_ip, uint16_t remote_port);
    
    void remove_connection(uint32_t remote_ip, uint16_t remote_port);
    void cleanup_timeout_connections();
    size_t get_connection_count() const;
    void set_max_connections(size_t max);

private:
    uint16_t allocate_local_port();  // 添加声明
    
    std::map<std::pair<uint32_t, uint16_t>, std::shared_ptr<TCPConnection>> connections_;
    mutable std::mutex mutex_;
    size_t max_connections_{1000};
    uint16_t next_port_{1024};  // 添加成员变量
    static constexpr size_t MAX_CONNECTIONS = 1000;
};

} // namespace lwip
