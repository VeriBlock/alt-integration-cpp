#include <gtest/gtest.h>

#include "util/literals.hpp"
#include "veriblock/entities/vbkblock.hpp"

using namespace VeriBlock;

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
  EXPECT_EQ(block.timestamp, defaultBlock.timestamp);
  EXPECT_EQ(block.difficulty, defaultBlock.difficulty);
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
  auto blockDecoded = ParseHex(defaultBlockEncoded);
  auto stream = ReadStream(blockDecoded);
  auto decoded = VbkBlock::fromVbkEncoding(stream);
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
  block.previousBlock = VBlakePrevBlockHash("94E7DC3E3BE21A96ECCF0FBD"_unhex);
  block.previousKeystone = VBlakePrevKeystoneHash("F5F62A3331DC995C36"_unhex);
  block.secondPreviousKeystone =
      VBlakePrevKeystoneHash("B0935637860679DDD5"_unhex);
  block.merkleRoot =
      VbkMerkleRootSha256Hash("67C9A83EF1B99B981ACBE73C1380F6DD"_unhex);
  block.timestamp = 1553699987;
  block.difficulty = 117586646;
  block.nonce = 1924857207;

  EXPECT_EQ(block.getBlockHash(),
            VBlakeBlockHash(
                "000000000000480D8196D5B0B41861D032377F5165BB4452"_unhex));
}
