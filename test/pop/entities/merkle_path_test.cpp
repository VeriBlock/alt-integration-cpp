// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/entities/merkle_path.hpp>
#include <veriblock/pop/literals.hpp>

using namespace altintegration;

static const std::string defaultPathEncoded =
    "02019f040000067b040000000c040000000400000020204d66077fdf24246ffd6b6979dfed"
    "ef5d46588654addeb35edb11e993c131f61220023d1abe8758c6f917ec0c65674bbd43d66e"
    "e14dc667b3117dfc44690c6f5af120096ddba03ca952af133fb06307c24171e53bf50ab76f"
    "1edeabde5e99f78d4ead202f32cf1bee50349d56fc1943af84f2d2abda520f64dc4db37b2f"
    "3db20b0ecb572093e70120f1b539d0c1495b368061129f30d35f9e436f32d69967ae86031a"
    "275620f554378a116e2142f9f6315a38b19bd8a1b2e6dc31201f2d37a058f03c39c06c2008"
    "24705685ceca003c95140434ee9d8bbbf4474b83fd4ecc2766137db9a44d7420b7b9e52f3e"
    "e8ce4fbb8be7d6cf66d33a20293f806c69385136662a74453fb162201732c9a35e80d4796b"
    "abea76aace50b49f6079ea3e349f026b4491cfe720ad17202d9b57e92ab51fe28a587050fd"
    "82abb30abd699a5ce8b54e7cd49b2a827bcb9920dcba229acdc6b7f028ba756fd5abbfebd3"
    "1b4227cd4137d728ec5ea56c457618202cf1439a6dbcc1a35e96574bddbf2c5db9174af5ad"
    "0d278fe92e06e4ac349a42";

static const std::vector<uint256> defaultLayers{
    "4d66077fdf24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f612"_unhex,
    "023d1abe8758c6f917ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af1"_unhex,
    "096ddba03ca952af133fb06307c24171e53bf50ab76f1edeabde5e99f78d4ead"_unhex,
    "2f32cf1bee50349d56fc1943af84f2d2abda520f64dc4db37b2f3db20b0ecb57"_unhex,
    "93e70120f1b539d0c1495b368061129f30d35f9e436f32d69967ae86031a2756"_unhex,
    "f554378a116e2142f9f6315a38b19bd8a1b2e6dc31201f2d37a058f03c39c06c"_unhex,
    "0824705685ceca003c95140434ee9d8bbbf4474b83fd4ecc2766137db9a44d74"_unhex,
    "b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a74453fb162"_unhex,
    "1732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad17"_unhex,
    "2d9b57e92ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb99"_unhex,
    "dcba229acdc6b7f028ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618"_unhex,
    "2cf1439a6dbcc1a35e96574bddbf2c5db9174af5ad0d278fe92e06e4ac349a42"_unhex};

static const std::string defaultSubject =
    "94e097b110ba3adbb7b6c4c599d31d675de7be6e722407410c08ef352be585f1";

static const int defaultIndex = 1659;

TEST(MerklePath, DeserializeFromVbkEncoding) {
  auto merklePath = ParseHex(defaultPathEncoded);
  auto subject = ParseHex(defaultSubject);
  auto stream = ReadStream(merklePath);
  ValidationState state;
  MerklePath decoded;
  ASSERT_TRUE(
      DeserializeFromVbkEncoding(stream, uint256(subject), decoded, state));
  EXPECT_EQ(decoded.index, defaultIndex);
  EXPECT_EQ(decoded.subject.toHex(), defaultSubject);
  EXPECT_EQ(decoded.layers, defaultLayers);

  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}

TEST(MerklePath, Serialize) {
  auto subject = ParseHex(defaultSubject);
  MerklePath path{defaultIndex, subject, defaultLayers};

  WriteStream stream;
  path.toVbkEncoding(stream);
  auto pathBytes = stream.data();
  auto pathEncoded = HexStr(pathBytes);

  EXPECT_EQ(pathEncoded, defaultPathEncoded);
}

TEST(MerklePath, RoundTrip) {
  auto merklePath = ParseHex(defaultPathEncoded);
  auto subject = ParseHex(defaultSubject);
  auto stream = ReadStream(merklePath);
  MerklePath decoded;
  ValidationState state;
  ASSERT_TRUE(
      DeserializeFromVbkEncoding(stream, uint256(subject), decoded, state));
  EXPECT_EQ(decoded.index, defaultIndex);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto pathBytes = outputStream.data();
  auto pathReEncoded = HexStr(pathBytes);

  EXPECT_EQ(pathReEncoded, defaultPathEncoded);
}

TEST(MerklePath, RoundTripNew) {
  auto merklePath = ParseHex(defaultPathEncoded);
  auto subject = ParseHex(defaultSubject);
  ReadStream stream(merklePath);
  MerklePath decoded;
  ValidationState state;
  bool ret = DeserializeFromVbkEncoding(stream, subject, decoded, state);
  ASSERT_TRUE(ret);
  EXPECT_TRUE(state.IsValid());
  EXPECT_EQ(decoded.index, defaultIndex);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto pathBytes = outputStream.data();
  auto pathReEncoded = HexStr(pathBytes);

  EXPECT_EQ(pathReEncoded, defaultPathEncoded);
}
