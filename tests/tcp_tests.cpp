#include <gtest/gtest.h>
#include "../src/tcp/tcp_packet.hpp"
#include "../src/tcp/tcp_connection.hpp"

namespace lwip {
namespace test {

TEST(TCPTest, PacketSerialization) {
    TCPPacket packet;
    std::vector<uint8_t> test_data{1, 2, 3, 4};
    
    // 测试序列化和反序列化
    auto serialized = packet.serialize();
    EXPECT_TRUE(packet.parse(serialized));
}

TEST(TCPTest, ConnectionStateMachine) {
    TCPConnection conn;
    EXPECT_TRUE(conn.connect(0x0100007F, 80));  // 连接到127.0.0.1:80
    
    // 测试状态转换
    std::vector<uint8_t> data{1, 2, 3, 4};
    EXPECT_TRUE(conn.send(data));
}

} // namespace test
} // namespace lwip
