#include "veriblock/entities/vbk_merkle_path.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

using namespace VeriBlock;

static const std::string defaultPathEncoded =
    "04000000010400000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375de"
    "bdc5ed22531c04000000022000000000000000000000000000000000000000000000000000"
    "00000000000000200000000000000000000000000000000000000000000000000000000000"
    "000000";

static const std::vector<Sha256Hash> defaultLayers{
    "0000000000000000000000000000000000000000000000000000000000000000"_unhex,
    "0000000000000000000000000000000000000000000000000000000000000000"_unhex};

static const std::string defaultSubject =
    "1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c";

static const int defaultTreeIndex = 1;
static const int defaultIndex = 0;

TEST(VbkMerklePath, Deserialize) {
  auto merklePath = ParseHex(defaultPathEncoded);
  auto stream = ReadStream(merklePath);
  auto decoded = VbkMerklePath::fromVbkEncoding(stream);

  EXPECT_EQ(decoded.treeIndex, defaultTreeIndex);
  EXPECT_EQ(decoded.index, defaultIndex);
  EXPECT_EQ(decoded.subject.toHex(), defaultSubject);
  EXPECT_EQ(decoded.layers, defaultLayers);

  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}

TEST(VbkMerklePath, Serialize) {
  auto subject = ParseHex(defaultSubject);
  VbkMerklePath path{defaultTreeIndex, defaultIndex, subject, defaultLayers};

  WriteStream stream;
  path.toVbkEncoding(stream);
  auto pathBytes = stream.data();
  auto pathEncoded = HexStr(pathBytes);

  EXPECT_EQ(pathEncoded, defaultPathEncoded);
}

TEST(VbkMerklePath, RoundTrip) {
  auto merklePath = ParseHex(defaultPathEncoded);
  auto stream = ReadStream(merklePath);
  auto decoded = VbkMerklePath::fromVbkEncoding(stream);
  EXPECT_EQ(decoded.index, defaultIndex);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto pathBytes = outputStream.data();
  auto pathReEncoded = HexStr(pathBytes);

  EXPECT_EQ(pathReEncoded, defaultPathEncoded);
}
