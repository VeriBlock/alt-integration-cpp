// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>

#include <string>
#include <util/test_utils.hpp>
#include <veriblock/pop/blockchain/block_index.hpp>
#include <veriblock/pop/entities/altblock.hpp>
#include <veriblock/pop/entities/btcblock.hpp>
#include <veriblock/pop/entities/popdata.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/storage/stored_block_index.hpp>

#include <veriblock/pop/literals.hpp>

static const std::string AltBlockIndexVbkEncoded =
    "00000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"
    "201aaaaaaaaaaaa9395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c000005ba"
    "0000009c000000000100010220010000000000000000000000000000000000000000000000"
    "00000000000000002002000000000000000000000000000000000000000000000000000000"
    "00000000010220030000000000000000000000000000000000000000000000000000000000"
    "00002004000000000000000000000000000000000000000000000000000000000000000102"
    "0c0500000000000000000000000c0600000000000000000000000100";

static const std::string VbkBlockIndexVbkEncoded =
    "0000339d00000000124b3f9c46bbefc75421502bd0ef5af409cf5f359194367f897099b11e"
    "67cc119c034af69b1eed67773bc2a4ce5074f900005c5400003d03000000647f0000000601"
    "00010000000064010120bb7a58a28645b393e8beaad00fef66e81c00c5577066b7582657e8"
    "fbe0819f770100";

static const std::string BtcBlockIndexVbkEncoded =
    "00000026de150000d011d4944e303862401b08298144c1f7dccdaa9a87b4686ddf3fde5d63"
    "a8493bf50b9b484e8e16fdc0d12b19329793f058b5adf131391976c4dd94d21dad15978509"
    "0000f6520000271e00000000000101000101000060c4";

using namespace altintegration;

static const AltBlock defaultBlock{
    "1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"_unhex,
    "1aaaaaaaaaaaa9395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"_unhex,
    156,
    1466};

TEST(BlockIndex, BTC) {
  AltChainParamsRegTest altparam{};
  StoredBlockIndex<BtcBlock> index;
  auto data = ParseHex(BtcBlockIndexVbkEncoded);
  ReadStream stream(data);
  ValidationState state;
  ASSERT_TRUE(DeserializeFromVbkEncoding(stream, index, state, altparam));
}

TEST(BlockIndex, VBK) {
  AltChainParamsRegTest altparam{};
  StoredBlockIndex<VbkBlock> index;
  auto data = ParseHex(VbkBlockIndexVbkEncoded);
  ReadStream stream(data);
  ValidationState state;
  ASSERT_TRUE(DeserializeFromVbkEncoding(stream, index, state, altparam));
}

TEST(BlockIndex, ALT) {
  AltChainParamsRegTest altparam{};
  StoredBlockIndex<AltBlock> index;
  auto data = ParseHex(AltBlockIndexVbkEncoded);
  ReadStream stream(data);
  ValidationState state;
  ASSERT_TRUE(DeserializeFromVbkEncoding(stream, index, state, altparam))
      << state.toString();
}

template <typename TestCase>
struct BlockIndexTest : public ::testing::Test {};

TYPED_TEST_SUITE_P(BlockIndexTest);

TYPED_TEST_P(BlockIndexTest, RoundTrip) {
  using Block = TypeParam;
  using Index = StoredBlockIndex<Block>;
  Index index = getRandomIndex<Block>();
  ValidationState state;
  AltChainParamsRegTest altparam{};

  auto vbkencoded = SerializeToVbkEncoding<Index>(index);
  Index decodedVbk;
  ASSERT_TRUE(
      DeserializeFromVbkEncoding<Index>(vbkencoded, decodedVbk, state, altparam))
      << state.toString();
  ASSERT_TRUE(state.IsValid());
  ASSERT_EQ(index.toVbkEncoding(), decodedVbk.toVbkEncoding());
}

// clang-format off
typedef ::testing::Types<
    BtcBlock,
    VbkBlock,
    AltBlock
> TypesUnderTest;
// clang-format on

REGISTER_TYPED_TEST_SUITE_P(BlockIndexTest, RoundTrip);

INSTANTIATE_TYPED_TEST_SUITE_P(BlockIndexTestSuite,
                               BlockIndexTest,
                               TypesUnderTest);

TEST(AltBlockIndex, IdsAreEqual) {
  AltChainParamsRegTest altparam{};
  StoredBlockIndex<AltBlock> index;
  index.header = std::make_shared<AltBlock>(defaultBlock);
  index.addon._atvids.push_back(uint256::fromHex("01"));
  index.addon._atvids.push_back(uint256::fromHex("02"));
  index.addon._vtbids.push_back(uint256::fromHex("03"));
  index.addon._vtbids.push_back(uint256::fromHex("04"));
  index.addon._vbkblockids.push_back(uint96::fromHex("05"));
  index.addon._vbkblockids.push_back(uint96::fromHex("06"));

  ValidationState state;
  StoredBlockIndex<AltBlock> after;
  auto hex = SerializeToHex(index);

  ASSERT_TRUE(DeserializeFromHex(hex, after, state, altparam));
  ASSERT_EQ(index.addon._atvids, after.addon._atvids);
  ASSERT_EQ(index.addon._vtbids, after.addon._vtbids);
  ASSERT_EQ(index.addon._vbkblockids, after.addon._vbkblockids);
}

TEST(VbkBlockIndex, IdsAreEqual) {
  AltChainParamsRegTest altparam{};
  StoredBlockIndex<VbkBlock> index;
  index.addon._vtbids.push_back(uint256::fromHex("01"));
  index.addon._vtbids.push_back(uint256::fromHex("02"));

  ValidationState state;
  StoredBlockIndex<VbkBlock> after;
  auto hex = SerializeToHex(index);

  ASSERT_TRUE(DeserializeFromHex(hex, after, state, altparam));
  ASSERT_EQ(index.addon._vtbids, after.addon._vtbids);
}
