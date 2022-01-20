// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/arith_uint256.hpp>
#include <veriblock/pop/blockchain/block_index.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/literals.hpp>

using namespace altintegration;

static const VbkBlock defaultBlock{5000,
                                   2,
                                   "449c60619294546ad825af03"_unhex,
                                   "b0935637860679ddd5"_unhex,
                                   "5ee4fd21082e18686e"_unhex,
                                   "26bbfda7d5e4462ef24ae02d67e47d78"_unhex,
                                   1553699059,
                                   16842752,
                                   1};

static const std::string defaultBlockEncoded =
    "41000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e"
    "26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001";

TEST(VbkBlock, DeserializeFromVbkEncoding) {
  auto block = AssertDeserializeFromHex<VbkBlock>(defaultBlockEncoded);

  EXPECT_EQ(block.getHeight(), defaultBlock.getHeight());
  EXPECT_EQ(block.getVersion(), defaultBlock.getVersion());
  EXPECT_EQ(block.getPreviousBlock().toHex(),
            defaultBlock.getPreviousBlock().toHex());
  EXPECT_EQ(block.getPreviousKeystone().toHex(),
            defaultBlock.getPreviousKeystone().toHex());
  EXPECT_EQ(block.getSecondPreviousKeystone().toHex(),
            defaultBlock.getSecondPreviousKeystone().toHex());
  EXPECT_EQ(block.getMerkleRoot().toHex(),
            defaultBlock.getMerkleRoot().toHex());
  EXPECT_EQ(block.getTimestamp(), defaultBlock.getTimestamp());
  EXPECT_EQ(block.getDifficulty(), defaultBlock.getDifficulty());
  EXPECT_EQ(block.getNonce(), defaultBlock.getNonce());
}

TEST(VbkBlock, Serialize) {
  WriteStream stream;
  defaultBlock.toVbkEncoding(stream);
  auto vbkBytes = stream.data();
  auto blockEncoded = HexStr(vbkBytes);
  EXPECT_EQ(blockEncoded, defaultBlockEncoded);
}

TEST(VbkBlock, RoundTrip) {
  auto decoded = AssertDeserializeFromHex<VbkBlock>(defaultBlockEncoded);
  EXPECT_EQ(decoded.getVersion(), defaultBlock.getVersion());

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto vbkBytes = outputStream.data();
  auto blockReEncoded = HexStr(vbkBytes);
  EXPECT_EQ(blockReEncoded, defaultBlockEncoded);
}

TEST(VbkBlock, RoundTripNew) {
  auto blockEncoded = ParseHex(defaultBlockEncoded);
  VbkBlock decoded;
  ValidationState state;
  bool ret = DeserializeFromVbkEncoding(blockEncoded, decoded, state);
  ASSERT_TRUE(ret);
  EXPECT_TRUE(state.IsValid());
  EXPECT_EQ(decoded.getVersion(), defaultBlock.getVersion());

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto vbkBytes = outputStream.data();
  auto blockReEncoded = HexStr(vbkBytes);
  EXPECT_EQ(blockReEncoded, defaultBlockEncoded);
}

TEST(VbkBlock, RoundTripWithHash) {
  const auto blockEncoded = defaultBlock.toRaw();
  const auto& hash = defaultBlock.getHash();
  VbkBlock block;
  ValidationState state;
  ReadStream stream(blockEncoded);
  EXPECT_TRUE(DeserializeFromRaw(stream, block, state, hash))
      << state.toString();
  EXPECT_EQ(block, defaultBlock);
}

TEST(VbkBlock, getBlockHash_test) {
  VbkBlock block;
  block.setHeight(5000);
  block.setVersion(2);
  block.setPreviousBlock(uint96("94E7DC3E3BE21A96ECCF0FBD"_unhex));
  block.setPreviousKeystone(uint72("F5F62A3331DC995C36"_unhex));
  block.setSecondPreviousKeystone(uint72("B0935637860679DDD5"_unhex));
  block.setMerkleRoot(uint128("DB0F135312B2C27867C9A83EF1B99B98"_unhex));
  block.setTimestamp(1553699987);
  block.setDifficulty(117586646);
  block.setNonce(1924857207);

  EXPECT_EQ(
      ArithUint256::fromLEBytes(block.getHash()),
      ArithUint256::fromHex(
          "00000000000000001f45c91342b8ac0ea7ae4d721be2445dc86ddc3f0e454f60"));
}

TEST(VbkBlock, getId_test) {
  auto vbkblock = AssertDeserializeFromHex<VbkBlock>(defaultBlockEncoded);
  EXPECT_EQ(vbkblock.getId().toHex(), "cd97599e23096ad42f119b5a");
}
