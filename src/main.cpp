#include "core/stack_manager.hpp"
#include "interface/network_interface.hpp"
#include "utils/logger.hpp"
#include "utils/config_parser.hpp"
#include <iostream>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    using namespace lwip;
    Logger::instance().set_level(LogLevel::DEBUG);
    LOG_INFO("Starting network stack...");

    // 加载配置
    StackConfig config;
    ConfigParser parser;
    if (!parser.parse_file("stack_config.conf", config)) {
        LOG_ERROR("Failed to load configuration");
        return 1;
    }

    // 初始化网络栈
    auto& stack = StackManager::instance();
    auto interface = std::make_unique<TunInterface>();
    
    if (!stack.initialize(std::move(interface))) {
        LOG_ERROR("Failed to initialize network stack");
        return 1;
    }

    // 启动网络栈
    stack.start();

    // 等待用户输入退出
    std::cout << "Network stack running. Press Enter to exit." << std::endl;
    std::cin.get();

    stack.stop();
    return 0;
}
