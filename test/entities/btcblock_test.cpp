#include "veriblock/entities/btcblock.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

using namespace VeriBlock;

TEST(BtcBlock, Deserialize) {
  auto block =
      "00000020f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e2213b75c841a011a2e00f29a"_unhex;
  auto stream = ReadStream(block);
  auto decoded = BtcBlock::fromRaw(stream);

  EXPECT_EQ(decoded.version, 536870912u);
  EXPECT_EQ(decoded.previousBlock.toHex(),
            "f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000");
  EXPECT_EQ(decoded.merkleRoot.toHex(),
            "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e");
  EXPECT_EQ(decoded.timestamp, 1555501858u);
  EXPECT_EQ(decoded.bits, 436279940u);
  EXPECT_EQ(decoded.nonce, (uint32_t)-1695416274);

  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}

TEST(BtcBlock, Serialize) {
  BtcBlock block;
  WriteStream stream;
  block.version = 536870912u;
  block.previousBlock =
      "f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"_unhex;
  block.merkleRoot =
      "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"_unhex;
  block.timestamp = 1555501858u;
  block.bits = 436279940u;
  block.nonce = (uint32_t)-1695416274;
  BtcBlock::toRaw(block, stream);

  std::vector<uint8_t> btcBytes = stream.data();
  std::string btcHex = HexStr<std::vector<uint8_t> >(btcBytes);
  EXPECT_EQ("00000020f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e2213b75c841a011a2e00f29a", btcHex);
}
