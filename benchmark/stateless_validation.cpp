// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <benchmark/benchmark.h>

#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/crypto/progpow.hpp>
#include <veriblock/entities/popdata.hpp>
#include <veriblock/literals.hpp>
#include <veriblock/pop_stateless_validator.hpp>
#include <veriblock/stateless_validation.hpp>
#include <veriblock/blockchain/miner.hpp>
#include <veriblock/bootstraps.hpp>

#include <iostream>

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
#define THREADS 2

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

static PopData pdclear = makePopData(TOTAL);
// popdata with VBK blocks with empty 'hash_'
static PopData pd = pdclear;

void warmupEthashCache() {
  VbkBlock block;
  block.setHeight(vbk.getProgPowForkHeight() + 0);
  block.setTimestamp(vbk.getProgPowStartTimeEpoch());
  block.calculateHash();
}

static void SerialPopDataValidation(benchmark::State& state) {
  warmupEthashCache();

  for (auto _ : state) {
    state.PauseTiming();
    pd = pdclear;
    ValidationState st;
    progpow::clearHeaderCache();
    state.ResumeTiming();

    // serially verify
    for (auto& block : pd.context) {
      if(!checkBlock(block, st, vbk)) {
        break;
      }
    }
  }
}

static void ParallelPopDataValidation(benchmark::State& state) {
  static PopValidator val(vbk, btc, alt, THREADS);
  warmupEthashCache();

  for (auto _ : state) {
    state.PauseTiming();
    pd = pdclear;
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
        VBK_ASSERT_MSG(s.GetPath() == "vbk-bad-pow", s.toString());
        if (!s.IsValid()) {
          break;
        }
      }
    }
  }
}

static void ParallelPopDataValidation2(benchmark::State& state) {
  static PopValidator val(vbk, btc, alt, THREADS);
  warmupEthashCache();

  for (auto _ : state) {
    state.PauseTiming();
    pd = pdclear;
    progpow::clearHeaderCache();
    ValidationState st;
    state.ResumeTiming();

    checkPopData(val, pd, st);
  }
}

// As a result of this benchmark, we can see that parallelized checkPopData is
// at least THREADS times faster than sequential version

BENCHMARK(SerialPopDataValidation)->Unit(benchmark::kMillisecond);
BENCHMARK(ParallelPopDataValidation)->Unit(benchmark::kMillisecond);
BENCHMARK(ParallelPopDataValidation2)->Unit(benchmark::kMillisecond);

// Run the benchmark
BENCHMARK_MAIN();
