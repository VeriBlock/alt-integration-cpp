// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/storage/inmem_payloads_provider.hpp>

using namespace altintegration;

struct AltChainParamsNon0Bootstrap : public AltChainParams {
  AltChainParamsNon0Bootstrap(int height) : id(1337), height(height) {}
  ~AltChainParamsNon0Bootstrap() override = default;

  AltBlock getBootstrapBlock() const noexcept override {
    AltBlock b;
    b.hash = std::vector<uint8_t>(12, 1);
    b.previousBlock = std::vector<uint8_t>(12, 0);
    b.height = height;
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
  int height = 0;
};

struct PositiveTest : public testing::TestWithParam<int> {};
TEST_P(PositiveTest, BootstrapSuccess) {
  int HEIGHT = GetParam();
  ASSERT_GE(HEIGHT, 0);

  AltChainParamsNon0Bootstrap alt(HEIGHT);
  VbkChainParamsRegTest vbk;
  BtcChainParamsRegTest btc;
  InmemPayloadsProvider pp;

  AltBlockTree tree(alt, vbk, btc, pp);
  ValidationState state;
  ASSERT_TRUE(tree.bootstrap(state));

  auto* tip = tree.getBestChain().tip();
  ASSERT_TRUE(tip);
  ASSERT_EQ(tip->getHeight(), HEIGHT);
}

INSTANTIATE_TEST_SUITE_P(
    AltBlockTree,
    PositiveTest,
    testing::Values(0, 1, 1337, std::numeric_limits<int32_t>::max() / 2 - 1));

struct NegativeTest : public testing::TestWithParam<int> {};
TEST_P(NegativeTest, BootstrapFail) {
  int HEIGHT = GetParam();

  AltChainParamsNon0Bootstrap alt(HEIGHT);
  VbkChainParamsRegTest vbk;
  BtcChainParamsRegTest btc;
  InmemPayloadsProvider pp;

  AltBlockTree tree(alt, vbk, btc, pp);
  ValidationState state;
  ASSERT_DEATH(
      {
        bool success = tree.bootstrap(state);
        (void)success;
      },
      "");
}

INSTANTIATE_TEST_SUITE_P(AltBlockTree,
                         NegativeTest,
                         testing::Values(-1,
                                         std::numeric_limits<int>::min(),
                                         std::numeric_limits<int>::max()));