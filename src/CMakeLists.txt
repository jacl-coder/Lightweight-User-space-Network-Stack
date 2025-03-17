add_subdirectory(core)
add_subdirectory(interface)
add_subdirectory(ip)
add_subdirectory(tcp)
add_subdirectory(udp)
add_subdirectory(utils)

# 核心库
add_library(lwip_core
    core/stack_manager.cpp
    core/event_loop.cpp
    core/dns_resolver.cpp
    core/stats_collector.cpp
)

# 接口库
add_library(lwip_interface
    interface/network_interface.cpp
    interface/arp_handler.cpp
)

# 工具库
add_library(lwip_utils
    utils/logger.cpp
    utils/config_parser.cpp
    utils/buffer_pool.cpp
    utils/performance_analyzer.cpp
)

# 设置包含目录
target_include_directories(lwip_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lwip_interface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lwip_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 链接依赖
target_link_libraries(lwip_core
    PRIVATE
    lwip_utils
    pthread
)

target_link_libraries(lwip_interface
    PRIVATE
    lwip_utils
)
