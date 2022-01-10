// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <benchmark/benchmark.h>

#include <veriblock/pop/literals.hpp>
#include <veriblock/pop/stateless_validation.hpp>

using namespace altintegration;

static void ProgpowEpochCached(benchmark::State& state) {
  VbkBlock block;
  block.setHeight(0);
  block.getHash();
  for (auto _ : state) {
    block.setHeight((block.getHeight() + 1) % 8000);
    benchmark::DoNotOptimize(block.getHash());
  }
}

static void ProgpowEpochUnCached(benchmark::State& state) {
  VbkBlock block;
  block.setHeight(8000);
  for (auto _ : state) {
    block.setHeight(block.getHeight() + 8000);
    benchmark::DoNotOptimize(block.getHash());
  }
}

BENCHMARK(ProgpowEpochCached)->Unit(benchmark::kMillisecond);;
BENCHMARK(ProgpowEpochUnCached)->Unit(benchmark::kMillisecond);;
// Run the benchmark
BENCHMARK_MAIN();
