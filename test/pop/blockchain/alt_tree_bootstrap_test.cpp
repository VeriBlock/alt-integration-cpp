// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/pop_test_fixture.hpp>
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/storage/adaptors/block_provider_impl.hpp>
#include <veriblock/pop/storage/adaptors/inmem_storage_impl.hpp>
#include <veriblock/pop/storage/adaptors/payloads_provider_impl.hpp>

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
                        const std::vector<uint8_t>&,
                        ValidationState&) const noexcept override {
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
  adaptors::InmemStorageImpl storage{};
  adaptors::PayloadsStorageImpl pp{storage};
  adaptors::BlockReaderImpl bp{storage, alt};

  AltBlockTree tree(alt, vbk, btc, pp, bp);
  ValidationState state;
  tree.bootstrap();

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
  adaptors::InmemStorageImpl storage{};
  adaptors::PayloadsStorageImpl pp{storage};
  adaptors::BlockReaderImpl bp{storage, alt};

  AltBlockTree tree(alt, vbk, btc, pp, bp);
  ValidationState state;
  ASSERT_DEATH({ tree.bootstrap(); }, "");
}

INSTANTIATE_TEST_SUITE_P(AltBlockTree,
                         NegativeTest,
                         testing::Values(-1,
                                         std::numeric_limits<int>::min(),
                                         std::numeric_limits<int>::max()));

struct AltBlockTreeTest : public testing::Test {};

TEST_F(AltBlockTreeTest, AssureBootstrapBtcBlockHasRefs_test) {
  using btc_block_tree = BlockTree<BtcBlock, BtcChainParams>;
  using vbk_block_tree = VbkBlockTree;

  ValidationState state;
  AltChainParamsRegTest altparam;
  BtcChainParamsRegTest btc_params{};
  VbkChainParamsRegTest vbk_params{};
  adaptors::InmemStorageImpl storage{};
  adaptors::PayloadsStorageImpl payloads_provider{storage};
  adaptors::BlockReaderImpl block_provider{storage, altparam};

  Miner<BtcBlock, BtcChainParams> btc_miner{btc_params};

  vbk_block_tree vbk_tree{
      vbk_params, btc_params, payloads_provider, block_provider};
  btc_block_tree& btc_tree = vbk_tree.btc();

  btc_tree.bootstrapWithGenesis(GetRegTestBtcBlock());
  auto* tip = btc_tree.getBestChain().tip();
  // bootstrap blocks have one ref always set
  ASSERT_GT(tip->getRefs().size(), 0);

  for (size_t i = 0; i < 10; i++) {
    BtcBlock block = btc_miner.createNextBlock(*tip);
    ASSERT_TRUE(btc_tree.acceptBlockHeader(block, state));
    tip = btc_tree.getBlockIndex(block.getHash());
    // when we mine without mock miner, there are no refs in blocks
    ASSERT_EQ(tip->getRefs().size(), 0);
  }
}
