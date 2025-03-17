#include "stack_manager.hpp"
#include "../utils/logger.hpp"

namespace lwip {

StackManager& StackManager::instance() {
    static StackManager instance;
    return instance;
}

StackManager::StackManager() : buffer_pool_(1500, 1024) {
    // 初始化默认配置
}

bool StackManager::initialize(std::unique_ptr<NetworkInterface> interface) {
    if (!interface) {
        LOG_ERROR("Invalid network interface");
        return false;
    }

    interface_ = std::move(interface);
    if (!interface_->init()) {
        LOG_ERROR("Failed to initialize network interface");
        return false;
    }

    LOG_INFO("Stack manager initialized successfully");
    return true;
}

void StackManager::start() {
    running_ = true;
    event_loop_.run();
    
    // 创建接收线程
    receive_thread_ = std::thread([this]() {
        while (running_) {
            std::vector<uint8_t> buffer;
            if (interface_->receive_frame(buffer)) {
                event_loop_.post([this, data = std::move(buffer)]() {
                    process_incoming_packet(data);
                });
            }
        }
    });
    
    LOG_INFO("Stack manager started");
}

void StackManager::stop() {
    running_ = false;
    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }
    event_loop_.stop();
    LOG_INFO("Stack manager stopped");
}

bool StackManager::send_packet(const std::vector<uint8_t>& packet) {
    return interface_->send_frame(packet);
}

void StackManager::process_incoming_packet(const std::vector<uint8_t>& packet) {
    if (packet.size() < 20) {
        LOG_ERROR("Packet too small");
        return;
    }

    // 增加调试日志
    LOG_DEBUG("Received packet of size: " + std::to_string(packet.size()));
    
    uint8_t protocol = packet[9];
    LOG_DEBUG("Protocol: " + std::to_string(protocol));

    switch (protocol) {
        case 1:  // ICMP
            handle_icmp_packet(packet);
            break;
        case 6:  // TCP
            handle_tcp_packet(packet);
            break;
        case 17: // UDP 
            handle_udp_packet(packet);
            break;
        default:
            LOG_WARN("Unsupported protocol: " + std::to_string(protocol));
            break;
    }
}

void StackManager::handle_icmp_packet(const std::vector<uint8_t>& packet) {
    // 解析IP头
    if (packet.size() < 20) return; // IP头最小长度
    
    size_t ip_header_len = (packet[0] & 0x0F) * 4;
    ICMPPacket icmp;
    std::vector<uint8_t> icmp_data(packet.begin() + ip_header_len, packet.end());
    
    if (!icmp.parse(icmp_data)) {
        LOG_ERROR("Failed to parse ICMP packet");
        return;
    }

    if (icmp.get_type() == 8) { // Echo Request
        ICMPPacket response;
        response.create_echo_reply(icmp);
        
        // 构建IP头
        std::vector<uint8_t> ip_response(packet.begin(), packet.begin() + ip_header_len);
        // 交换源目地址
        std::swap(ip_response[12], ip_response[16]);
        std::swap(ip_response[13], ip_response[17]);
        std::swap(ip_response[14], ip_response[18]);
        std::swap(ip_response[15], ip_response[19]);
        
        auto icmp_response = response.serialize();
        ip_response.insert(ip_response.end(), icmp_response.begin(), icmp_response.end());
        
        send_packet(ip_response);
        LOG_INFO("ICMP Echo reply sent");
    }
}

void StackManager::handle_tcp_packet(const std::vector<uint8_t>& packet) {
    size_t ip_header_len = (packet[0] & 0x0F) * 4;
    std::vector<uint8_t> tcp_data(packet.begin() + ip_header_len, packet.end());
    
    TCPPacket tcp;
    if (!tcp.parse(tcp_data)) {
        LOG_ERROR("Failed to parse TCP packet");
        return;
    }

    auto connection = tcp_manager_.find_or_create_connection(
        ntohl(*(uint32_t*)&packet[16]),  // source ip
        tcp.get_source_port()
    );
    
    if (connection) {
        connection->handle_packet(tcp);
    }
}

void StackManager::handle_udp_packet(const std::vector<uint8_t>& packet) {
    size_t ip_header_len = (packet[0] & 0x0F) * 4;
    std::vector<uint8_t> udp_data(packet.begin() + ip_header_len, packet.end());
    
    UDPPacket udp;
    if (!udp.parse(udp_data)) {
        LOG_ERROR("Failed to parse UDP packet");
        return;
    }

    // 处理DNS查询等UDP服务
    if (udp.get_dest_port() == 53) {
        dns_resolver_.handle_query(udp);
    }
}

} // namespace lwip
