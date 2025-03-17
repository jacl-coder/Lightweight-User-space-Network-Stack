add_library(udp OBJECT
    udp_packet.cpp
)

target_include_directories(udp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
