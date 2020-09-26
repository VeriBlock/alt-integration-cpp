// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <benchmark/benchmark.h>

#include <veriblock/literals.hpp>
#include <veriblock/stateless_validation.hpp>

using namespace altintegration;

static void ProgpowEpochCached(benchmark::State& state) {
  VbkBlock block;
  block.setHeight(0);
  block.calculateHash();
  for (auto _ : state) {
    block.setHeight((block.getHeight() + 1) % 8000);
    benchmark::DoNotOptimize(block.calculateHash());
  }
}

static void ProgpowEpochUnCached(benchmark::State& state) {
  VbkBlock block;
  block.setHeight(8000);
  for (auto _ : state) {
    block.setHeight(block.getHeight() + 8000);
    benchmark::DoNotOptimize(block.calculateHash());
  }
}

BENCHMARK(ProgpowEpochCached)->Unit(benchmark::kMillisecond);;
BENCHMARK(ProgpowEpochUnCached)->Unit(benchmark::kMillisecond);;
// Run the benchmark
BENCHMARK_MAIN();
