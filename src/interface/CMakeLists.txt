add_library(interface OBJECT
    network_interface.cpp
    non_blocking_interface.cpp
    arp_handler.cpp
)

target_include_directories(interface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(interface PRIVATE utils)
