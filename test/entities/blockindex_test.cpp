// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>

#include <string>
#include <util/test_utils.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/popdata.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/serde.hpp>

static const std::string AltBlockIndexVbkEncoded =
    "0000006600000020de1046c3a19258aa7964f655d291502b9bcf02aa7e9ddb9542c77fb8f4"
    "ca82d20000002054b1c57ce5914c9409e60c4cdc6d9bfb59097fd60fae9b3117a5ebea762d"
    "4bca00000066000000660000016001000100010120e5bc64c81bb00ac853022ee4cd33560b"
    "f351af6700cf4767ee2b64cd240ae67701150c7272f1db71a75c040c19214d0c55162625a7"
    "2855a1559606fb0cbdd3ecff62045dae767eb5a70c677faaf298168148d2fb9e740c4d358f"
    "043831066e2483ef860cf2d8b4f5c30e4e5fcd1782ca0c30256c889fa25fb437c9c1c80c63"
    "1ea82dd64b8e09391dbd5e0c4766061bf7e1ea2987e7340d0c886de74a9639190f1bee350d"
    "0c80ef33351c75f021ba04dbaf0c61c6218d0e89351834af871c0c54cfcf428c3674a88429"
    "900b0c038861cd80085db0a1de6efb0c06476879bdd745763c9f80640cdda3988184b18c67"
    "47ce7a390cf76a730e84c535dd5814b0570c620234afc35a64b7b3fa39040c6b616a753312"
    "89070ea41db30c6f849eee87863e2677a257590c5591714cd94807de9977cd52";

static const std::string VbkBlockIndexVbkEncoded =
    "00000043000000430002dd5179ee90f2d59ab819ab1cf068644a619592388530fbe25f14df"
    "653a9ead29e41ee6c8d135c0a3c845b180dde35f8c848b0101000000000000000000016000"
    "00000101012084c5ef4592fdc67038e69ebfa32a52ff6c81baa82301744281a270dfab8f82"
    "fb1850e9e995583c09170c7d9510ba4c980c0c8ec126530ba7a918b2323cec803c693d0596"
    "370e155cee0bcb2e82ee6120d91120166d85b68da59f567cbed28a664f5ce96592d953e0ce"
    "fed8f8e14acb63fd17190001012084c5ef4592fdc67038e69ebfa32a52ff6c81baa8230174"
    "4281a270dfab8f82fb";

static const std::string BtcBlockIndexVbkEncoded =
    "000000010100000006226e46111a0b59caaf126043eb5bbf28c34f3a5e332a1fc7b2b73cf1"
    "88910fa1dba9aa0a3ef1eadb813f49d8910589eda0d01213d0e469da78301da0f72d8a8b84"
    "8c5fffff7f2000000000000000c0010100000043";

using namespace altintegration;

TEST(BlockIndex, BTC) {
  BlockIndex<BtcBlock> index;
  auto data = ParseHex(BtcBlockIndexVbkEncoded);
  ReadStream stream(data);
  ValidationState state;
  ASSERT_TRUE(DeserializeFromVbkEncoding(stream, index, state));
}

TEST(BlockIndex, VBK) {
  BlockIndex<VbkBlock> index;
  auto data = ParseHex(VbkBlockIndexVbkEncoded);
  ReadStream stream(data);
  ValidationState state;
  ASSERT_TRUE(DeserializeFromVbkEncoding(stream, index, state));
}

TEST(BlockIndex, ALT) {
  BlockIndex<AltBlock> index;
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
  using Index = BlockIndex<Block>;
  Index index = getRandomIndex<Block>();
  ValidationState state;

  auto vbkencoded = SerializeToVbkEncoding<Index>(index);
  Index decodedVbk;
  ASSERT_TRUE(DeserializeFromVbkEncoding<Index>(vbkencoded, decodedVbk, state));
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
