add_library(ip OBJECT
    ip_packet.cpp
    ipv6_packet.cpp
    ip_fragmenter.cpp
    icmp_packet.cpp
    ipv6_nd.cpp
)

target_include_directories(ip PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ip PRIVATE utils)
