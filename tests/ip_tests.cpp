#include <gtest/gtest.h>
#include "../src/ip/ip_packet.hpp"
#include "../src/ip/ipv6_packet.hpp"
#include "../src/ip/ip_fragmenter.hpp"

namespace lwip {
namespace test {

TEST(IPTest, BasicPacketHandling) {
    IPPacket packet;
    std::vector<uint8_t> test_data{1, 2, 3, 4};
    packet.set_payload(test_data);
    
    auto serialized = packet.serialize();
    EXPECT_TRUE(packet.parse(serialized));
}

TEST(IPTest, Fragmentation) {
    IPFragmenter fragmenter;
    IPPacket packet;
    std::vector<uint8_t> large_data(2000, 'A');
    packet.set_payload(large_data);
    
    auto fragments = fragmenter.fragment_packet(packet, 1500);
    EXPECT_GT(fragments.size(), 1);
}

} // namespace test
} // namespace lwip
