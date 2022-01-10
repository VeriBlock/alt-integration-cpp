// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <benchmark/benchmark.h>

#include <veriblock/pop/crypto/progpow.hpp>

using namespace altintegration;

static void Keccakf800(benchmark::State& state) {
  std::array<uint32_t, 25> st;
  st.fill(0);

  for (auto _ : state) {
    progpow::keccak_f800(st);
  }
}

BENCHMARK(Keccakf800);
// Run the benchmark
BENCHMARK_MAIN();
