# 添加子目录
add_subdirectory(core)
add_subdirectory(interface)
add_subdirectory(ip)
add_subdirectory(tcp)
add_subdirectory(udp)
add_subdirectory(utils)

# 将所有组件作为 OBJECT 库组合到主库
add_library(lwip_stack
    $<TARGET_OBJECTS:core>
    $<TARGET_OBJECTS:interface>
    $<TARGET_OBJECTS:ip>
    $<TARGET_OBJECTS:tcp>
    $<TARGET_OBJECTS:udp>
    $<TARGET_OBJECTS:utils>
)

# 设置目标的包含目录
target_include_directories(lwip_stack 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接所需的系统库
target_link_libraries(lwip_stack
    PRIVATE
    pthread
)
