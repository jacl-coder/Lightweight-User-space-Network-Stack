find_package(benchmark QUIET)

if(benchmark_FOUND)
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
else()
    message(STATUS "Google Benchmark not found - benchmarks will not be built")
endif()
