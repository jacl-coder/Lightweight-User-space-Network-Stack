#include "dns_resolver.hpp"
#include "../utils/logger.hpp"
#include <arpa/inet.h>
#include <cstring>

namespace lwip {

DNSResolver& DNSResolver::instance() {
    static DNSResolver instance;
    return instance;
}

std::future<InetAddress> DNSResolver::resolve(const std::string& hostname) {
    return std::async(std::launch::async, [this, hostname]() {
        // 创建UDP socket
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            LOG_ERROR("Failed to create DNS socket");
            return InetAddress();
        }

        // 构建DNS请求
        std::vector<uint8_t> request(512);
        DNSHeader* header = reinterpret_cast<DNSHeader*>(request.data());
        header->id = htons(rand() & 0xFFFF);
        header->flags = htons(0x0100);  // 标准查询
        header->questions = htons(1);
        header->answers = 0;
        header->authority = 0;
        header->additional = 0;

        // 设置超时
        struct timeval tv;
        tv.tv_sec = timeout_.count() / 1000;
        tv.tv_usec = (timeout_.count() % 1000) * 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        // 发送查询到所有DNS服务器
        for (const auto& server : dns_servers_) {
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(53);
            addr.sin_addr.s_addr = htonl(server.get_ip());

            if (sendto(sock, request.data(), request.size(), 0,
                      (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                continue;
            }

            // 接收响应
            std::vector<uint8_t> response(512);
            if (recvfrom(sock, response.data(), response.size(), 0, nullptr, nullptr) > 0) {
                // 解析响应...
                // 这里简化处理，实际实现需要完整解析DNS响应报文
                uint32_t ip = ntohl(*reinterpret_cast<uint32_t*>(response.data() + 12));
                close(sock);
                return InetAddress(ip);
            }
        }

        close(sock);
        return InetAddress();
    });
}

void DNSResolver::add_dns_server(const InetAddress& server) {
    dns_servers_.push_back(server);
}

void DNSResolver::set_timeout(std::chrono::milliseconds timeout) {
    timeout_ = timeout;
}

} // namespace lwip
