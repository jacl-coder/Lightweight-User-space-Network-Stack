#include <gtest/gtest.h>
#include "../src/core/dns_resolver.hpp"

namespace lwip {
namespace test {

TEST(DNSTest, BasicResolution) {
    auto& resolver = DNSResolver::instance();
    resolver.add_dns_server(InetAddress("8.8.8.8", 53));
    
    auto future = resolver.resolve("example.com");
    auto result = future.get();
    EXPECT_NE(result.get_ip(), 0);
}

} // namespace test
} // namespace lwip
