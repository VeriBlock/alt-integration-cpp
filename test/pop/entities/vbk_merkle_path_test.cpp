// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/entities/vbk_merkle_path.hpp>
#include <veriblock/pop/literals.hpp>

using namespace altintegration;

static const std::string defaultPathEncoded =
    "04000000010400000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375de"
    "bdc5ed22531c04000000022000000000000000000000000000000000000000000000000000"
    "00000000000000200000000000000000000000000000000000000000000000000000000000"
    "000000";

static const std::vector<uint256> defaultLayers{
    "0000000000000000000000000000000000000000000000000000000000000000"_unhex,
    "0000000000000000000000000000000000000000000000000000000000000000"_unhex};

static const std::string defaultSubject =
    "1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c";

static const int defaultTreeIndex = 1;
static const int defaultIndex = 0;

TEST(VbkMerklePath, DeserializeFromVbkEncoding) {
  auto decoded = AssertDeserializeFromHex<VbkMerklePath>(defaultPathEncoded);

  EXPECT_EQ(decoded.treeIndex, defaultTreeIndex);
  EXPECT_EQ(decoded.index, defaultIndex);
  EXPECT_EQ(decoded.subject.toHex(), defaultSubject);
  EXPECT_EQ(decoded.layers, defaultLayers);
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
  auto decoded = AssertDeserializeFromHex<VbkMerklePath>(defaultPathEncoded);
  EXPECT_EQ(decoded.index, defaultIndex);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto pathBytes = outputStream.data();
  auto pathReEncoded = HexStr(pathBytes);

  EXPECT_EQ(pathReEncoded, defaultPathEncoded);
}

TEST(VbkMerklePath, RoundTripNew) {
  auto merklePath = ParseHex(defaultPathEncoded);
  VbkMerklePath decoded;
  ValidationState state;
  bool ret = DeserializeFromVbkEncoding(merklePath, decoded, state);
  ASSERT_TRUE(ret);
  EXPECT_TRUE(state.IsValid());
  EXPECT_EQ(decoded.index, defaultIndex);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto pathBytes = outputStream.data();
  auto pathReEncoded = HexStr(pathBytes);

  EXPECT_EQ(pathReEncoded, defaultPathEncoded);
}
