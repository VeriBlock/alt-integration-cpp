#include "veriblock/entities/btcblock.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

using namespace VeriBlock;

static const BtcBlock defaultBlock{
    536870912u,
    "f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"_unhex,
    "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"_unhex,
    1555501858u,
    436279940u,
    (uint32_t)-1695416274};

static const std::string defaultBlockEncoded =
    "00000020f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"
    "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e2213b75c"
    "841a011a2e00f29a";

TEST(BtcBlock, Deserialize) {
  auto blockEncoded = ParseHex(defaultBlockEncoded);
  auto stream = ReadStream(blockEncoded);
  auto decoded = BtcBlock::fromRaw(stream);

  EXPECT_EQ(decoded.version, defaultBlock.version);
  EXPECT_EQ(decoded.previousBlock.toHex(), defaultBlock.previousBlock.toHex());
  EXPECT_EQ(decoded.merkleRoot.toHex(), defaultBlock.merkleRoot.toHex());
  EXPECT_EQ(decoded.timestamp, defaultBlock.timestamp);
  EXPECT_EQ(decoded.bits, defaultBlock.bits);
  EXPECT_EQ(decoded.nonce, defaultBlock.nonce);

  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}

TEST(BtcBlock, Serialize) {
  WriteStream stream;
  BtcBlock::toRaw(stream, defaultBlock);
  auto btcBytes = stream.data();
  auto blockEncoded = HexStr(btcBytes);
  EXPECT_EQ(blockEncoded, defaultBlockEncoded);
}

TEST(BtcBlock, RoundTrip) {
  auto blockDecoded = ParseHex(defaultBlockEncoded);
  auto stream = ReadStream(blockDecoded);
  auto decoded = BtcBlock::fromRaw(stream);
  EXPECT_EQ(decoded.version, defaultBlock.version);

  WriteStream outputStream;
  BtcBlock::toRaw(outputStream, decoded);
  auto btcBytes = outputStream.data();
  auto blockReEncoded = HexStr(btcBytes);
  EXPECT_EQ(blockReEncoded, defaultBlockEncoded);
}
