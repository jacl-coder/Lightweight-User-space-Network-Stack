add_library(utils OBJECT
    logger.cpp
    buffer_pool.cpp
    config_parser.cpp
    performance_analyzer.cpp
    performance_monitor.cpp
    packet_capture.cpp
    zero_copy_buffer.cpp
)

target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
