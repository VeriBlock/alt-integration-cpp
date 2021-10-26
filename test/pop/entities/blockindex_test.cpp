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

static const std::string AltBlockIndexVbkEncoded =
    "000031c70cfbe6c1cd7ceb5e36cba675560c76bc289cc7a324cff40777980000406d000022"
    "960000000601000102200192219fd88238e59cb1760dc881461c5cc5be252dc98b6e7f942c"
    "ced5695f59200894d94097633af23e724087ca2b3b5f2b8ec106cb2ea25b1e7c1ef27b4856"
    "b6010220a3415c17f0bc012706e77b07ba0e760729b10048038886da2ac5ff217c99677d20"
    "ecf96f29d8a27364975baccf517aa71713a9f5d322ea25b090d9efcb0e31b25e01020c2a85"
    "01d3be4d60d3a42574060cfd816e28a98b673ea42051c10100";

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

TEST(BlockIndex, BTC) {
  StoredBlockIndex<BtcBlock> index;
  auto data = ParseHex(BtcBlockIndexVbkEncoded);
  ReadStream stream(data);
  ValidationState state;
  ASSERT_TRUE(DeserializeFromVbkEncoding(stream, index, state));
}

TEST(BlockIndex, VBK) {
  StoredBlockIndex<VbkBlock> index;
  auto data = ParseHex(VbkBlockIndexVbkEncoded);
  ReadStream stream(data);
  ValidationState state;
  ASSERT_TRUE(DeserializeFromVbkEncoding(stream, index, state));
}

TEST(BlockIndex, ALT) {
  StoredBlockIndex<AltBlock> index;
  auto data = ParseHex(AltBlockIndexVbkEncoded);
  ReadStream stream(data);
  ValidationState state;
  ASSERT_TRUE(DeserializeFromVbkEncoding(stream, index, state))
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

  auto vbkencoded = SerializeToVbkEncoding<Index>(index);
  Index decodedVbk;
  ASSERT_TRUE(DeserializeFromVbkEncoding<Index>(vbkencoded, decodedVbk, state))
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
  StoredBlockIndex<AltBlock> index;
  index.addon._atvids.push_back(uint256::fromHex("01"));
  index.addon._atvids.push_back(uint256::fromHex("02"));
  index.addon._vtbids.push_back(uint256::fromHex("03"));
  index.addon._vtbids.push_back(uint256::fromHex("04"));
  index.addon._vbkblockids.push_back(uint96::fromHex("05"));
  index.addon._vbkblockids.push_back(uint96::fromHex("06"));

  ValidationState state;
  StoredBlockIndex<AltBlock> after;
  auto hex = SerializeToHex(index);

  ASSERT_TRUE(DeserializeFromHex(hex, after, state));
  ASSERT_EQ(index.addon._atvids, after.addon._atvids);
  ASSERT_EQ(index.addon._vtbids, after.addon._vtbids);
  ASSERT_EQ(index.addon._vbkblockids, after.addon._vbkblockids);
}

TEST(VbkBlockIndex, IdsAreEqual) {
  StoredBlockIndex<VbkBlock> index;
  index.addon._vtbids.push_back(uint256::fromHex("01"));
  index.addon._vtbids.push_back(uint256::fromHex("02"));

  ValidationState state;
  StoredBlockIndex<VbkBlock> after;
  auto hex = SerializeToHex(index);

  ASSERT_TRUE(DeserializeFromHex(hex, after, state));
  ASSERT_EQ(index.addon._vtbids, after.addon._vtbids);
}