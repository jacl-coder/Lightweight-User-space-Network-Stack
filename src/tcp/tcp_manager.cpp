#include "tcp_manager.hpp"
#include "../utils/logger.hpp"
#include <algorithm>

namespace lwip {

TCPManager::TCPManager() {
    // 初始化配置
    max_connections_ = 1000;  // 可以从配置文件读取
}

std::shared_ptr<TCPConnection> TCPManager::create_connection() {
    if (connections_.size() >= MAX_CONNECTIONS) {
        LOG_ERROR("Maximum TCP connections reached");
        return nullptr;
    }
    
    auto conn = std::make_shared<TCPConnection>();
    uint16_t local_port = allocate_local_port();
    conn->set_local_port(local_port);
    return conn;
}

std::shared_ptr<TCPConnection> TCPManager::find_or_create_connection(
    uint32_t remote_ip, uint16_t remote_port) {
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找已存在的连接
    auto key = std::make_pair(remote_ip, remote_port);
    auto it = connections_.find(key);
    if (it != connections_.end()) {
        return it->second;
    }
    
    // 创建新连接
    if (connections_.size() < max_connections_) {
        auto conn = std::make_shared<TCPConnection>();
        uint16_t local_port = allocate_local_port();
        conn->set_local_port(local_port);
        connections_[key] = conn;
        LOG_INFO("Created new TCP connection from " + 
                 std::to_string(remote_ip) + ":" + 
                 std::to_string(remote_port));
        return conn;
    }
    
    LOG_ERROR("Maximum TCP connections reached");
    return nullptr;
}

void TCPManager::remove_connection(uint32_t remote_ip, uint16_t remote_port) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto key = std::make_pair(remote_ip, remote_port);
    if (connections_.erase(key) > 0) {
        LOG_INFO("Removed TCP connection from " + 
                 std::to_string(remote_ip) + ":" + 
                 std::to_string(remote_port));
    }
}

void TCPManager::cleanup_timeout_connections() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = connections_.begin(); it != connections_.end();) {
        if (it->second->is_timeout()) {
            LOG_INFO("Removing timed out TCP connection from " + 
                     std::to_string(it->first.first) + ":" + 
                     std::to_string(it->first.second));
            it = connections_.erase(it);
        } else {
            ++it;
        }
    }
}

size_t TCPManager::get_connection_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connections_.size();
}

void TCPManager::set_max_connections(size_t max) {
    max_connections_ = max;
}

uint16_t TCPManager::allocate_local_port() {
    while (true) {
        uint16_t port = next_port_++;
        if (port < 1024) next_port_ = 1024;
        
        // 检查端口是否已使用
        bool used = false;
        for (const auto& [key, conn] : connections_) {
            if (conn->get_local_port() == port) {
                used = true;
                break;
            }
        }
        
        if (!used) return port;
    }
}

} // namespace lwip
