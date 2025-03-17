#include <gtest/gtest.h>
#include "../src/core/stack_manager.hpp"
#include "../src/interface/network_interface.hpp"

TEST(BasicTest, StackInitialization) {
    auto& stack = lwip::StackManager::instance();
    auto interface = std::make_unique<lwip::TunInterface>();
    
    EXPECT_TRUE(stack.initialize(std::move(interface)));
    
    stack.start();
    // 简单的ping测试
    // TODO: 实现ping测试
    stack.stop();
}
