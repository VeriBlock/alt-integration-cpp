// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <benchmark/benchmark.h>

#include <iostream>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/miner.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/bootstraps.hpp>
#include <veriblock/crypto/progpow.hpp>
#include <veriblock/entities/popdata.hpp>
#include <veriblock/literals.hpp>
#include <veriblock/pop_stateless_validator.hpp>
#include <veriblock/stateless_validation.hpp>

using namespace altintegration;

struct AltChainParamsImpl : public AltChainParams {
  ~AltChainParamsImpl() override = default;

  AltChainParamsImpl() { mMaxVbkBlocksInAltBlock = 5000; }

  AltBlock getBootstrapBlock() const noexcept override {
    AltBlock b;
    b.hash = std::vector<uint8_t>(12, 1);
    b.previousBlock = std::vector<uint8_t>(12, 0);
    b.height = 0;
    b.timestamp = 0;
    return b;
  }

  int64_t getIdentifier() const noexcept override { return id; }

  std::vector<uint8_t> getHash(
      const std::vector<uint8_t>& bytes) const noexcept override {
    return AssertDeserializeFromRaw<AltBlock>(bytes).getHash();
  }

  bool checkBlockHeader(const std::vector<uint8_t>&,
                        const std::vector<uint8_t>&) const noexcept override {
    return true;
  }

  int64_t id = 0;
};

static BtcChainParamsRegTest btc;
static VbkChainParamsRegTest vbk;
static AltChainParamsImpl alt;
#define TOTAL 30
#define THREADS 4

PopData makePopData(int blocks) {
  Miner<VbkBlock, VbkChainParams> miner(vbk);
  PopData pd;
  pd.version = 1;

  auto last = GetRegTestVbkBlock();

  for (int i = 0; i < blocks; i++) {
    VbkBlock dummy;
    dummy.setDifficulty(last.getDifficulty());
    dummy.setHeight(vbk.getProgPowForkHeight() + i);
    dummy.setTimestamp(vbk.getProgPowStartTimeEpoch() +
                       i * vbk.getRetargetPeriod());
    miner.createBlock(dummy);
    pd.context.push_back(dummy);
  }
  return pd;
}

static PopData pd;

void warmupEthashCache() {
  VbkBlock block;
  block.setHeight(vbk.getProgPowForkHeight() + 0);
  block.setTimestamp(vbk.getProgPowStartTimeEpoch());
  block.getHash();
}

static void SerialPopDataValidation(benchmark::State& state) {
  warmupEthashCache();
  pd = makePopData(TOTAL);

  for (auto _ : state) {
    state.PauseTiming();
    ValidationState st;
    progpow::clearHeaderCache();
    state.ResumeTiming();

    // serially verify
    for (auto& block : pd.context) {
      checkBlock(block, st, vbk);
      VBK_ASSERT(st.IsValid());
    }
  }
}

static void ParallelPopDataValidation(benchmark::State& state) {
  static PopValidator val(vbk, btc, alt, THREADS);
  warmupEthashCache();
  pd = makePopData(TOTAL);

  for (auto _ : state) {
    state.PauseTiming();
    progpow::clearHeaderCache();
    ValidationState st;
    state.ResumeTiming();

    {
      std::vector<std::future<ValidationState>> f;
      f.reserve(pd.context.size());
      for (auto& block : pd.context) {
        f.push_back(val.addCheck(block));
      }
      for (auto& fut : f) {
        ValidationState s = fut.get();
        VBK_ASSERT(s.IsValid());
      }
    }
  }
}

static void ParallelPopDataValidation2(benchmark::State& state) {
  static PopValidator val(vbk, btc, alt, THREADS);
  warmupEthashCache();
  pd = makePopData(TOTAL);

  for (auto _ : state) {
    state.PauseTiming();
    progpow::clearHeaderCache();
    ValidationState st;
    state.ResumeTiming();

    checkPopData(val, pd, st);
    VBK_ASSERT(st.IsValid());
  }
}

// As a result of this benchmark, we can see that parallelized checkPopData is
// at least THREADS times faster than sequential version

BENCHMARK(SerialPopDataValidation)->Unit(benchmark::kMillisecond);
BENCHMARK(ParallelPopDataValidation)->Unit(benchmark::kMillisecond);
BENCHMARK(ParallelPopDataValidation2)->Unit(benchmark::kMillisecond);

// Run the benchmark
BENCHMARK_MAIN();
