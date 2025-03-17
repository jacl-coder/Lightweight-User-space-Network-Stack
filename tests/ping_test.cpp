#include <gtest/gtest.h>
#include "../src/core/stack_manager.hpp"
#include "../src/ip/icmp_packet.hpp"

TEST(NetworkTest, PingTest) {
    auto& stack = lwip::StackManager::instance();
    
    // 创建 ICMP echo request
    lwip::ICMPPacket ping;
    ping.create_echo_request(1, 1);
    
    // 发送并等待响应
    EXPECT_TRUE(stack.send_packet(ping.serialize()));
    
    // 等待响应
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 检查是否收到响应
    // TODO: 实现响应检查
}
