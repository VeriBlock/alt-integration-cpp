// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <benchmark/benchmark.h>

#include <veriblock/pop/cache/small_lfru_cache.hpp>
#include <veriblock/pop/third_party/lru_cache.hpp>

using namespace altintegration;

const int SIZE = 5;
const int K = 300;

static void StaticCache(benchmark::State& state) {
  srand(0);
  cache::SmallLFRUCache<int, int, SIZE> cache;

  auto ptr = std::shared_ptr<int>();
  for (auto _ : state) {
    int key = rand() % K;
    std::shared_ptr<int> value =
        cache.getOrDefault(key, [] { return std::make_shared<int>(rand()); });
  }
}

static void ThirdPartyCache(benchmark::State& state) {
  srand(0);
  lru11::Cache<int, std::shared_ptr<int>, std::mutex> cache(SIZE);

  for (auto _ : state) {
    int key = rand() % K;
    std::shared_ptr<int> value;
    if (!cache.tryGet(key, value)) {
      value = std::make_shared<int>(rand());
      cache.insert(key, value);
    }
  }
}

BENCHMARK(StaticCache);
BENCHMARK(ThirdPartyCache);
// Run the benchmark
BENCHMARK_MAIN();
