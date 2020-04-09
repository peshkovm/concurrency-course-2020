#include <benchmark/benchmark.h>

#include "cyclic_barrier.hpp"

static const size_t kThreads = 16;

solutions::CyclicBarrier barrier{kThreads};

static void BM_Runners(benchmark::State& state) {
  for (auto _ : state) {
    barrier.Arrive();
  }
}

BENCHMARK(BM_Runners)
    ->MinTime(5)
    ->UseRealTime()
    ->Threads(kThreads);

BENCHMARK_MAIN();

