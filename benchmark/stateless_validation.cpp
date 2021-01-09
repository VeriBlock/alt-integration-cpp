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

#include "../test/util/alt_chain_params_regtest.hpp"

using namespace altintegration;

struct AltChainParamsImpl : public AltChainParams {
  ~AltChainParamsImpl() override = default;

  AltChainParamsImpl() { mMaxVbkBlocksInAltBlock = 9999; }

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

PopData makePopData(int blocks) {
  PopData pd;
  pd.version = 1;

  for (int i = 0; i < blocks; i++) {
    VbkBlock block;
    block.setHeight(vbk.getProgPowForkHeight() + i);
    block.setTimestamp(vbk.getProgPowStartTimeEpoch() +
                       i * vbk.getRetargetPeriod());

    pd.context.push_back(block);
  }
  return pd;
}

void precalcCache() {
  VbkBlock block;
  block.setHeight(vbk.getProgPowForkHeight() + 0);
  block.setTimestamp(vbk.getProgPowStartTimeEpoch());
  block.calculateHash();
}

#define TOTAL 100

static void SerialPopDataValidation(benchmark::State& state) {
  precalcCache();

  for (auto _ : state) {
    state.PauseTiming();
    ValidationState st;
    const auto pd = makePopData(TOTAL);
    progpow::clearHeaderCache();
    state.ResumeTiming();

    // serially verify
    for (auto& block : pd.context) {
      checkBlock(block, st, vbk);
    }
  }
}

static void ParallelPopDataValidation(benchmark::State& state) {
  static PopValidator val(vbk, btc, alt, 2);
  precalcCache();

  for (auto _ : state) {
    state.PauseTiming();
    const auto pd = makePopData(TOTAL);
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
      }
    }
  }
}

static void ParallelPopDataValidation2(benchmark::State& state) {
  static PopValidator val(vbk, btc, alt, 2);
  precalcCache();

  for (auto _ : state) {
    state.PauseTiming();
    const auto pd = makePopData(TOTAL);
    progpow::clearHeaderCache();
    ValidationState st;
    state.ResumeTiming();

    {
      checkPopData(val, pd, st);
    }
  }
}

BENCHMARK(SerialPopDataValidation)->Unit(benchmark::kMillisecond);
BENCHMARK(ParallelPopDataValidation)->Unit(benchmark::kMillisecond);
BENCHMARK(ParallelPopDataValidation2)->Unit(benchmark::kMillisecond);

// Run the benchmark
BENCHMARK_MAIN();
