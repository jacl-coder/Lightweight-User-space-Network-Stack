enable_testing()
find_package(GTest REQUIRED)

add_executable(stack_tests
    ip_tests.cpp
    tcp_tests.cpp
    udp_tests.cpp
    arp_tests.cpp
)

target_link_libraries(stack_tests
    PRIVATE
    GTest::GTest
    GTest::Main
    lwip_stack
)

add_test(NAME stack_tests COMMAND stack_tests)
