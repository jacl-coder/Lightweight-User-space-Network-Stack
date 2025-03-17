add_library(core
    stack_manager.cpp
    event_loop.cpp
    dns_resolver.cpp
    stats_collector.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
