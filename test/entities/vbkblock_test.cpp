#include "veriblock/entities/vbkblock.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

using namespace VeriBlock;

TEST(VbkBlock, Deserialize) {
  const auto vbkblock =
      "40000013880002449C60619294546AD825AF03B0935637860679DDD55EE4FD21082E18686E26BBFDA7D5E4462EF24AE02D67E47D785C9B90F30101000000000001"_unhex;
  auto stream = ReadStream(vbkblock);
  auto block = VbkBlock::fromVbkEncoding(stream);

  EXPECT_EQ(block.height, 5000);
  EXPECT_EQ(block.version, 2);
  EXPECT_EQ(block.previousBlock.toHex(), "449c60619294546ad825af03");
  EXPECT_EQ(block.previousKeystone.toHex(), "b0935637860679ddd5");
  EXPECT_EQ(block.secondPreviousKeystone.toHex(), "5ee4fd21082e18686e");
  EXPECT_EQ(block.merkleRoot.toHex(), "26bbfda7d5e4462ef24ae02d67e47d78");
  EXPECT_EQ(block.timestamp, 1553699059);
  EXPECT_EQ(block.difficulty, 16842752);
  EXPECT_EQ(block.nonce, 1);
}