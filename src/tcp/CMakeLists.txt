add_library(tcp OBJECT
    tcp_packet.cpp
    tcp_connection.cpp
    tcp_manager.cpp
    tcp_retransmitter.cpp
)

target_include_directories(tcp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tcp PRIVATE utils)
