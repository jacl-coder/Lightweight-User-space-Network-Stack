enable_testing()
find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

# 定义所有测试源文件
set(TEST_SOURCES
    basic_test.cpp
    functional_test.cpp
    tcp_tests.cpp
    udp_tests.cpp
    ip_tests.cpp
    dns_tests.cpp
    ping_test.cpp
)

# 添加基准测试作为可选项
option(ENABLE_BENCHMARKS "Enable benchmark tests" OFF)
if(ENABLE_BENCHMARKS)
    add_subdirectory(benchmark)
endif()

# 创建测试可执行文件
add_executable(stack_tests ${TEST_SOURCES})

# 链接必要的库
target_link_libraries(stack_tests
    PRIVATE
    GTest::GTest
    GTest::Main
    lwip_stack
    pthread
)

# 添加到CTest
add_test(NAME unit_tests COMMAND stack_tests)
