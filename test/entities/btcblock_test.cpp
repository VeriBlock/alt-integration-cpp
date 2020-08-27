// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/btcblock.hpp"

#include <gtest/gtest.h>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/literals.hpp"

using namespace altintegration;

static const BtcBlock defaultBlock{
    536870912u,
    "f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"_unhex,
    "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"_unhex,
    1555501858u,
    436279940u,
    (uint32_t)-1695416274};

static const std::string defaultBlockEncoded =
    "0000002000000000000000b345b7bbf29bda1507a679b97967f99a10ab0088899529def75e"
    "16e6cef738a2eba1fe7409318e3f558bec325392427aa3d8eaf46b028654f82213b75c841a"
    "011a2e00f29a";

TEST(BtcBlock, Deserialize) {
  auto blockEncoded = ParseHex(defaultBlockEncoded);
  auto stream = ReadStream(blockEncoded);
  auto decoded = BtcBlock::fromRaw(stream);

  EXPECT_EQ(decoded.version, defaultBlock.version);
  EXPECT_EQ(decoded.previousBlock.toHex(), defaultBlock.previousBlock.toHex());
  EXPECT_EQ(decoded.merkleRoot.toHex(), defaultBlock.merkleRoot.toHex());
  EXPECT_EQ(decoded.getBlockTime(), defaultBlock.getBlockTime());
  EXPECT_EQ(decoded.getDifficulty(), defaultBlock.getDifficulty());
  EXPECT_EQ(decoded.nonce, defaultBlock.nonce);

  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}

TEST(BtcBlock, Serialize) {
  WriteStream stream;
  defaultBlock.toRaw(stream);
  auto btcBytes = stream.data();
  auto blockEncoded = HexStr(btcBytes);
  EXPECT_EQ(blockEncoded, defaultBlockEncoded);
}

TEST(BtcBlock, RoundTrip) {
  auto blockEncoded = ParseHex(defaultBlockEncoded);
  auto stream = ReadStream(blockEncoded);
  auto decoded = BtcBlock::fromRaw(stream);
  EXPECT_EQ(decoded.version, defaultBlock.version);

  WriteStream outputStream;
  decoded.toRaw(outputStream);
  auto btcBytes = outputStream.data();
  auto blockReEncoded = HexStr(btcBytes);
  EXPECT_EQ(blockReEncoded, defaultBlockEncoded);
}

TEST(BtcBlock, RoundTripNew) {
  auto blockEncoded = ParseHex(defaultBlockEncoded);
  BtcBlock decoded;
  ValidationState state;
  bool ret = DeserializeRaw(blockEncoded, decoded, state);
  ASSERT_TRUE(ret);
  EXPECT_TRUE(state.IsValid());
  EXPECT_EQ(decoded.version, defaultBlock.version);

  WriteStream outputStream;
  decoded.toRaw(outputStream);
  auto btcBytes = outputStream.data();
  auto blockReEncoded = HexStr(btcBytes);
  EXPECT_EQ(blockReEncoded, defaultBlockEncoded);
}

TEST(BtcBlock, getBlockHash_test) {
  BtcBlock block;
  block.version = 536870912;
  block.previousBlock = uint256(
      "00000000000000b345b7bbf29bda1507a679b97967f99a10ab0088899529def7"_unhex);
  block.merkleRoot = uint256(
      "5e16e6cef738a2eba1fe7409318e3f558bec325392427aa3d8eaf46b028654f8"_unhex);
  block.timestamp = 1555501858;
  block.bits = 436279940;
  block.nonce = 2599551022;

  EXPECT_EQ(
      ArithUint256::fromLEBytes(block.getHash()),
      ArithUint256::fromHex(
          "000000000000000246200f09b513e517a3bd8c591a3b692d9852ddf1ee0f8b3a"));
}
