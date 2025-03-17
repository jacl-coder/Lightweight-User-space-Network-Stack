#pragma once
#include <memory>
#include "../interface/network_interface.hpp"
#include "../utils/buffer_pool.hpp"
#include "event_loop.hpp"
#include "../tcp/tcp_manager.hpp"
#include "../core/dns_resolver.hpp"

namespace lwip {

class StackManager {
public:
    static StackManager& instance();
    
    bool initialize(std::unique_ptr<NetworkInterface> interface);
    void start();
    void stop();
    
    EventLoop& get_event_loop() { return event_loop_; }
    BufferPool& get_buffer_pool() { return buffer_pool_; }

private:
    StackManager();
    
    void handle_icmp_packet(const std::vector<uint8_t>& packet);
    void handle_tcp_packet(const std::vector<uint8_t>& packet);
    void handle_udp_packet(const std::vector<uint8_t>& packet);
    
    std::thread receive_thread_;
    std::atomic<bool> running_{false};
    
    EventLoop event_loop_;
    BufferPool buffer_pool_{1500, 1024};  // MTU size, initial pool size
    std::unique_ptr<NetworkInterface> interface_;
    TCPManager tcp_manager_;
    DNSResolver dns_resolver_;
    bool send_packet(const std::vector<uint8_t>& packet);
    void process_incoming_packet(const std::vector<uint8_t>& packet);
};

} // namespace lwip
