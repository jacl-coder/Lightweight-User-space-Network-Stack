cmake_minimum_required(VERSION 3.10)
project(lwip_stack)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译选项
if(UNIX)
    add_compile_options(-Wall -Wextra)
endif()

# 添加子目录库
add_library(lwip_utils
    src/utils/logger.cpp
    src/utils/inet_address.cpp  # 添加这一行
)

add_library(lwip_core
    src/core/stack_manager.cpp
    src/core/dns_resolver.cpp
    src/core/event_loop.cpp
    src/utils/buffer_pool.cpp
    src/utils/config_parser.cpp
    src/ip/icmp_packet.cpp
    src/tcp/tcp_packet.cpp
    src/tcp/tcp_manager.cpp
    src/tcp/tcp_connection.cpp
    src/udp/udp_packet.cpp
)

add_library(lwip_interface
    src/interface/network_interface.cpp
)

# 设置库的包含目录
target_include_directories(lwip_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 添加头文件包含路径
target_include_directories(lwip_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 确保所有库都能访问头文件
target_include_directories(lwip_interface PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 添加依赖关系
target_link_libraries(lwip_core
    PUBLIC
    lwip_utils
)

target_link_libraries(lwip_interface
    PUBLIC
    lwip_utils
)

# 添加子目录
add_subdirectory(src)
add_subdirectory(tests)

# 主程序
add_executable(stack_main src/main.cpp)

# 修改主程序的链接顺序
target_link_libraries(stack_main
    PRIVATE
    lwip_interface
    lwip_core
    lwip_utils  # lwip_utils 需要放在最后，因为它被其他库依赖
)

# 安装目标
install(TARGETS stack_main
    RUNTIME DESTINATION bin
)

# 安装配置文件
install(FILES stack_config.conf
    DESTINATION etc
)
