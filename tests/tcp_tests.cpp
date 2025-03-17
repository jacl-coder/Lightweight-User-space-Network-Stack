#include <gtest/gtest.h>
#include "../src/tcp/tcp_packet.hpp"
#include "../src/tcp/tcp_connection.hpp"
#include "../src/tcp/tcp_manager.hpp"

namespace lwip {
namespace test {

class TCPTest : public ::testing::Test {
protected:
    TCPManager tcp_manager;
};

TEST_F(TCPTest, PacketHandling) {
    TCPPacket packet;
    std::vector<uint8_t> test_data{1, 2, 3, 4};
    
    // 测试TCP选项设置
    packet.set_mss(1460);
    packet.set_window_scale(7);
    packet.enable_timestamps(true);
    
    // 序列化和反序列化
    auto serialized = packet.serialize();
    EXPECT_TRUE(packet.parse(serialized));
    
    // 检查校验和
    EXPECT_TRUE(packet.verify_checksum());
}

TEST_F(TCPTest, ConnectionState) {
    auto conn = tcp_manager.create_connection();
    ASSERT_NE(conn, nullptr);
    
    // 测试TCP状态转换
    EXPECT_TRUE(conn->connect(0x0100007F, 80));
    EXPECT_TRUE(conn->is_connected());
    
    // 测试发送接收
    std::vector<uint8_t> send_data{1, 2, 3, 4};
    EXPECT_TRUE(conn->send(send_data));
    
    std::vector<uint8_t> recv_data;
    EXPECT_TRUE(conn->receive(recv_data));
    
    // 测试关闭
    conn->close();
    EXPECT_FALSE(conn->is_connected());
}

TEST_F(TCPTest, FlowControl) {
    auto conn = tcp_manager.create_connection();
    ASSERT_NE(conn, nullptr);
    
    // 测试流量控制
    conn->set_window_size(8192);
    std::vector<uint8_t> large_data(16384, 'X');
    EXPECT_TRUE(conn->send(large_data));  // 应该分片发送
}

} // namespace test
} // namespace lwip
