// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/vbkblock.hpp"

#include <gtest/gtest.h>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/literals.hpp"

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
    "40000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e"
    "26bbfda7d5e4462ef24ae02d67e47d785c9b90f30101000000000001";

TEST(VbkBlock, Deserialize) {
  const auto vbkblock = ParseHex(defaultBlockEncoded);
  auto stream = ReadStream(vbkblock);
  auto block = VbkBlock::fromVbkEncoding(stream);

  EXPECT_EQ(block.height, defaultBlock.height);
  EXPECT_EQ(block.version, defaultBlock.version);
  EXPECT_EQ(block.previousBlock.toHex(), defaultBlock.previousBlock.toHex());
  EXPECT_EQ(block.previousKeystone.toHex(),
            defaultBlock.previousKeystone.toHex());
  EXPECT_EQ(block.secondPreviousKeystone.toHex(),
            defaultBlock.secondPreviousKeystone.toHex());
  EXPECT_EQ(block.merkleRoot.toHex(), defaultBlock.merkleRoot.toHex());
  EXPECT_EQ(block.getBlockTime(), defaultBlock.getBlockTime());
  EXPECT_EQ(block.getDifficulty(), defaultBlock.getDifficulty());
  EXPECT_EQ(block.nonce, defaultBlock.nonce);
}

TEST(VbkBlock, Serialize) {
  WriteStream stream;
  defaultBlock.toVbkEncoding(stream);
  auto vbkBytes = stream.data();
  auto blockEncoded = HexStr(vbkBytes);
  EXPECT_EQ(blockEncoded, defaultBlockEncoded);
}

TEST(VbkBlock, RoundTrip) {
  auto blockEncoded = ParseHex(defaultBlockEncoded);
  auto stream = ReadStream(blockEncoded);
  auto decoded = VbkBlock::fromVbkEncoding(stream);
  EXPECT_EQ(decoded.version, defaultBlock.version);

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
  bool ret = Deserialize(blockEncoded, decoded, state);
  ASSERT_TRUE(ret);
  EXPECT_TRUE(state.IsValid());
  EXPECT_EQ(decoded.version, defaultBlock.version);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto vbkBytes = outputStream.data();
  auto blockReEncoded = HexStr(vbkBytes);
  EXPECT_EQ(blockReEncoded, defaultBlockEncoded);
}

TEST(VbkBlock, getBlockHash_test) {
  VbkBlock block;
  block.height = 5000;
  block.version = 2;
  block.previousBlock = uint96("94E7DC3E3BE21A96ECCF0FBD"_unhex);
  block.previousKeystone = uint72("F5F62A3331DC995C36"_unhex);
  block.secondPreviousKeystone = uint72("B0935637860679DDD5"_unhex);
  block.merkleRoot = uint128("DB0F135312B2C27867C9A83EF1B99B98"_unhex);
  block.timestamp = 1553699987;
  block.difficulty = 117586646;
  block.nonce = 1924857207;

  EXPECT_EQ(
      ArithUint256::fromLEBytes(block.getHash()),
      ArithUint256::fromHex(
          "0000000000000000000000000000480D8196D5B0B41861D032377F5165BB4452"));
}

TEST(VbkBlock, getId_test) {
  auto atvBytes = ParseHex(defaultBlockEncoded);
  auto stream = ReadStream(atvBytes);
  auto vbkblock = VbkBlock::fromVbkEncoding(stream);

  EXPECT_EQ(vbkblock.getId().toHex(), "08e2aae9a5e19569b1a68624");
}
