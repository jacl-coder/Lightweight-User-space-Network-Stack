cmake_minimum_required(VERSION 3.10)
project(lwip_stack)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译选项
if(UNIX)
    add_compile_options(-Wall -Wextra)
endif()

# 添加子目录
add_subdirectory(src)
add_subdirectory(tests)

# 主程序
add_executable(stack_main src/main.cpp)
target_link_libraries(stack_main
    PRIVATE
    lwip_core
    lwip_interface
    lwip_utils
)

# 安装目标
install(TARGETS stack_main
    RUNTIME DESTINATION bin
)

# 安装配置文件
install(FILES stack_config.conf
    DESTINATION etc
)
