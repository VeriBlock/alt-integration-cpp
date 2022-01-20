// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/entities/atv.hpp>
#include <veriblock/pop/literals.hpp>

#include "pop/util/test_utils.hpp"

using namespace altintegration;

static const PublicationData publicationData{
    0, "header bytes"_v, "payout info bytes"_v, "context info bytes"_v};

static const auto defaultSignature =
    "30440220398b74708dc8f8aee68fce0c47b8959e6fce6354665da3ed87a83f708e62a"
    "a6b02202e6c00c00487763c55e92c7b8e1dd538b7375d8df2b2117e75acbb9db7deb3"
    "c7"_unhex;

static const auto defaultPublicKey =
    "3056301006072a8648ce3d020106052b8104000a03420004de4ee8300c3cd99e91353"
    "6cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692b1d36eac7187950620"
    "a28838da60a8c9dd60190c14c59b82cb90319e"_unhex;

static const VbkTx defaultTx{
    NetworkBytePair{{false, 0}, (uint8_t)TxType::VBK_TX},
    Address::assertFromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"),
    Coin(1000),
    std::vector<Output>{},
    7,
    publicationData,
    defaultSignature,
    defaultPublicKey};

static const VbkMerklePath defaultPath{
    1,
    0,
    uint256(
        "1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"_unhex),
    std::vector<uint256>{
        uint256(
            "0000000000000000000000000000000000000000000000000000000000000000"_unhex),
        uint256(
            "0000000000000000000000000000000000000000000000000000000000000000"_unhex)}};

static const VbkBlock defaultVbkBlock{5000,
                                      2,
                                      "449c60619294546ad825af03"_unhex,
                                      "b0935637860679ddd5"_unhex,
                                      "5ee4fd21082e18686e"_unhex,
                                      "26bbfda7d5e4462ef24ae02d67e47d78"_unhex,
                                      1553699059,
                                      16842752,
                                      1};

static const ATV defaultAtv{1, defaultTx, defaultPath, defaultVbkBlock};

TEST(ATV, DeserializeFromVbkEncoding) {
  auto decoded = AssertDeserializeFromHex<ATV>(defaultAtvEncoded);

  EXPECT_EQ(decoded.transaction.sourceAddress,
            Address::assertFromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"));
}

TEST(ATV, Serialize) {
  WriteStream stream;
  defaultAtv.toVbkEncoding(stream);
  auto atvBytes = stream.data();
  auto atvEncoded = HexStr(atvBytes);
  EXPECT_EQ(atvEncoded, defaultAtvEncoded);
}

TEST(ATV, RoundTrip) {
  auto decoded = AssertDeserializeFromHex<ATV>(defaultAtvEncoded);
  EXPECT_EQ(decoded.transaction.sourceAddress,
            Address::assertFromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"));

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto txBytes = outputStream.data();
  auto txReEncoded = HexStr(txBytes);
  EXPECT_EQ(txReEncoded, defaultAtvEncoded);
}

TEST(ATV, RoundTripNew) {
  auto atvBytes = ParseHex(defaultAtvEncoded);
  ATV decoded;
  ValidationState state;
  bool ret = DeserializeFromVbkEncoding(atvBytes, decoded, state);
  ASSERT_TRUE(ret);
  EXPECT_TRUE(state.IsValid());
  EXPECT_EQ(decoded.transaction.sourceAddress,
            Address::assertFromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"));

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto txBytes = outputStream.data();
  auto txReEncoded = HexStr(txBytes);
  EXPECT_EQ(txReEncoded, defaultAtvEncoded);
}

TEST(ATV, getId_test) {
  auto atv = AssertDeserializeFromHex<ATV>(defaultAtvEncoded);

  EXPECT_EQ(atv.getId().toHex(),
            "c6d96b8e87f3e347aa1d1051bb3af39c8ea60612ced905d11c6f92d7b6bd50f5");
}
