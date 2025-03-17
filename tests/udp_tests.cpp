#include <gtest/gtest.h>
#include "../src/udp/udp_packet.hpp"
#include "../src/core/stack_manager.hpp"
#include "../src/utils/logger.hpp"  // Added include for Logger and LogLevel

namespace lwip {
namespace test {

class UDPTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::instance().set_level(LogLevel::DEBUG);
    }
};

TEST_F(UDPTest, PacketHandling) {
    UDPPacket packet;
    
    // 设置基本参数
    packet.set_source_port(12345);
    packet.set_dest_port(53);
    
    // 测试数据
    std::vector<uint8_t> test_data{1, 2, 3, 4, 5};
    packet.set_payload(test_data);
    
    // 序列化和反序列化测试
    auto serialized = packet.serialize();
    UDPPacket parsed_packet;
    EXPECT_TRUE(parsed_packet.parse(serialized));
    
    // 验证字段
    EXPECT_EQ(parsed_packet.get_source_port(), 12345);
    EXPECT_EQ(parsed_packet.get_dest_port(), 53);
    EXPECT_EQ(parsed_packet.get_payload(), test_data);
}

TEST_F(UDPTest, ChecksumVerification) {
    UDPPacket packet;
    packet.set_source_port(1234);
    packet.set_dest_port(5678);
    std::vector<uint8_t> data{'t', 'e', 's', 't'};
    packet.set_payload(data);
    
    // 计算和验证校验和
    uint16_t checksum = packet.calculate_checksum();
    EXPECT_NE(checksum, 0);
    EXPECT_TRUE(packet.verify_checksum());
}

TEST_F(UDPTest, LargePacketHandling) {
    UDPPacket packet;
    // 创建接近最大UDP数据包大小的数据
    std::vector<uint8_t> large_data(65507, 'A');  // Max UDP payload size
    
    packet.set_payload(large_data);
    auto serialized = packet.serialize();
    
    EXPECT_TRUE(packet.parse(serialized));
    EXPECT_EQ(packet.get_payload().size(), large_data.size());
}

} // namespace test
} // namespace lwip
