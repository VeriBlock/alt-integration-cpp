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
  block.height = 0;
  block.calculateHash();
  for (auto _ : state) {
    block.height = (1 + block.height) % 8000;
    benchmark::DoNotOptimize(block.calculateHash());
  }
}

static void ProgpowEpochUnCached(benchmark::State& state) {
  VbkBlock block;
  block.height = 8000;
  for (auto _ : state) {
    block.height += 8000;
    benchmark::DoNotOptimize(block.calculateHash());
  }
}

BENCHMARK(ProgpowEpochCached);
BENCHMARK(ProgpowEpochUnCached);
// Run the benchmark
BENCHMARK_MAIN();
