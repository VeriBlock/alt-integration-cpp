// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <benchmark/benchmark.h>

#include <veriblock/pop/mempool.hpp>
#include <veriblock/pop/storage/adaptors/block_provider_impl.hpp>
#include <veriblock/pop/storage/adaptors/inmem_storage_impl.hpp>
#include <veriblock/pop/storage/adaptors/payloads_provider_impl.hpp>
#include <veriblock/pop/mock_miner.hpp>

using namespace altintegration;

struct MempoolFixture: public benchmark::Fixture {
  MempoolFixture() {
    auto BTCgenesis = GetRegTestBtcBlock();
    auto VBKgenesis = GetRegTestVbkBlock();

    // by default, set mocktime to the latest time between all genesis blocks
    auto time = std::max({altparam.getBootstrapBlock().getTimestamp(),
                          BTCgenesis.getTimestamp(),
                          VBKgenesis.getTimestamp()});
    setMockTime(time + 1);

    alttree.btc().bootstrapWithGenesis(BTCgenesis);
    alttree.vbk().bootstrapWithGenesis(VBKgenesis);
    alttree.bootstrap();
  }

  void SetUp(const ::benchmark::State&) {
    // generate 100 vbk blocks
    for(int i = 0; i < 100; i++) {
      auto block = popminer.mineVbkBlocks(1);
      assert(mempool.submit(block->getHeader(), false, val_state));
    }
  }

  void TearDown(const ::benchmark::State&) {
  }

  AltChainParamsRegTest altparam{};
  VbkChainParamsRegTest vbkparam{};
  BtcChainParamsRegTest btcparam{};
  adaptors::InmemStorageImpl storage{};
  adaptors::PayloadsStorageImpl payloadsProvider{storage};
  adaptors::BlockReaderImpl blockProvider{storage, altparam};

  // miners
  MockMiner popminer{altparam, vbkparam, btcparam};

  // trees
  AltBlockTree alttree{
      altparam, vbkparam, btcparam, payloadsProvider, blockProvider};

  MemPool mempool{alttree};
  ValidationState val_state;
};

BENCHMARK_DEFINE_F(MempoolFixture, Mempool_generatePopData)(benchmark::State& state) {
    for (auto _ : state) {
      auto pop_data = mempool.generatePopData();
      (void)pop_data;
    }
}

BENCHMARK_REGISTER_F(MempoolFixture, Mempool_generatePopData);
// Run the benchmark
BENCHMARK_MAIN();