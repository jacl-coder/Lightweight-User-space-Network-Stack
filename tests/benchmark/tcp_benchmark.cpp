#include <benchmark/benchmark.h>
#include "../../src/tcp/tcp_connection.hpp"

namespace lwip {
namespace benchmark {

static void BM_TCPThroughput(benchmark::State& state) {
    TCPConnection conn;
    std::vector<uint8_t> data(state.range(0));
    
    for (auto _ : state) {
        conn.send(data);
    }
    
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}

BENCHMARK(BM_TCPThroughput)->Range(1<<10, 1<<20);

} // namespace benchmark
} // namespace lwip
