find_package(benchmark REQUIRED)

add_executable(stack_benchmarks
    tcp_benchmark.cpp
    udp_benchmark.cpp
    throughput_benchmark.cpp
)

target_link_libraries(stack_benchmarks
    PRIVATE
    benchmark::benchmark
    lwip_stack
)
