#include <gtest/gtest.h>
#include "../src/core/stack_manager.hpp"
#include "../src/interface/network_interface.hpp"
#include "../src/utils/logger.hpp"
#include "../src/tcp/tcp_connection.hpp"
#include "../src/udp/udp_packet.hpp"
#include "../src/core/performance_monitor.hpp"

namespace lwip {
namespace test {

class StackFunctionalTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::instance().set_level(LogLevel::DEBUG);
        auto& stack = StackManager::instance();
        auto interface = std::make_unique<TunInterface>();
        ASSERT_TRUE(stack.initialize(std::move(interface)));
        stack.start();
    }

    void TearDown() override {
        StackManager::instance().stop();
    }
};

TEST_F(StackFunctionalTest, BasicConnectivity) {
    // TCP连接测试
    TCPConnection tcp_conn;
    EXPECT_TRUE(tcp_conn.connect(0x0100007F, 80));  // 127.0.0.1:80

    // UDP数据包测试
    UDPPacket udp;
    std::vector<uint8_t> test_data{1, 2, 3, 4};
    auto serialized = udp.serialize();
    EXPECT_TRUE(udp.parse(serialized));

    // ICMP回显测试
    ICMPPacket ping;
    ping.create_echo_request(1, 1);
    auto& stack = StackManager::instance();
    EXPECT_TRUE(stack.send_packet(ping.serialize()));
}

TEST_F(StackFunctionalTest, PerformanceBasics) {
    // 缓冲池测试
    BufferPool pool(1500, 10);
    auto buffer = pool.acquire();
    EXPECT_NE(buffer, nullptr);

    // 性能监控测试
    auto& monitor = PerformanceMonitor::instance();
    monitor.record_throughput(1024);
    auto report = monitor.get_performance_report();
    EXPECT_FALSE(report.empty());
}

TEST_F(StackFunctionalTest, TCPConnectionTest) {
    // 创建TCP连接
    auto& tcp_manager = StackManager::instance().get_tcp_manager();
    auto conn = tcp_manager.create_connection();
    ASSERT_NE(conn, nullptr);

    // 设置TCP选项
    conn->set_mss(1460);
    conn->enable_timestamps(true);
    conn->enable_nagle(false);
    conn->set_window_size(65535);

    // 测试连接建立
    EXPECT_TRUE(conn->connect(0x0100007F, 8080));  // localhost:8080
    EXPECT_TRUE(conn->is_connected());

    // 测试数据发送
    std::vector<uint8_t> test_data(4096, 'A');
    EXPECT_TRUE(conn->send(test_data));

    // 测试数据接收
    std::vector<uint8_t> recv_data;
    EXPECT_TRUE(conn->receive(recv_data));

    // 测试连接关闭
    conn->close();
    EXPECT_FALSE(conn->is_connected());
}

TEST_F(StackFunctionalTest, TCPCongestionControl) {
    auto conn = std::make_shared<TCPConnection>();
    ASSERT_NE(conn, nullptr);

    // 设置初始窗口
    conn->set_window_size(16384);  // 16KB初始窗口

    // 模拟数据传输
    std::vector<uint8_t> large_data(1024 * 1024, 'B');  // 1MB数据
    EXPECT_TRUE(conn->send(large_data));

    // 验证拥塞控制状态
    // TODO: 添加状态检查方法
    
    // 模拟丢包重传
    conn->handle_timeout();
}

TEST_F(StackFunctionalTest, TCPMultipleConnections) {
    auto& tcp_manager = StackManager::instance().get_tcp_manager();
    std::vector<std::shared_ptr<TCPConnection>> connections;

    // 创建多个连接
    for (int i = 0; i < 5; ++i) {
        auto conn = tcp_manager.create_connection();
        ASSERT_NE(conn, nullptr);
        EXPECT_TRUE(conn->connect(0x0100007F, 8080 + i));
        connections.push_back(conn);
    }

    // 验证连接数量
    EXPECT_EQ(tcp_manager.get_connection_count(), 5);

    // 并发发送数据
    std::vector<uint8_t> test_data(1024, 'C');
    for (auto& conn : connections) {
        EXPECT_TRUE(conn->send(test_data));
    }

    // 关闭所有连接
    for (auto& conn : connections) {
        conn->close();
    }
}

} // namespace test
} // namespace lwip
