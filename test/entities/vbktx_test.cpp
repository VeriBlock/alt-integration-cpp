// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/vbktx.hpp"

#include <gtest/gtest.h>

#include "veriblock/literals.hpp"

using namespace altintegration;

static const PublicationData publicationData{0, ""_v, ""_v, ""_v};
static const std::vector<uint8_t> emptyBytes64(64);

static const VbkTx defaultTx{
    NetworkBytePair{false, 0, (uint8_t)TxType::VBK_TX},
    Address::assertFromString("V8dy5tWcP7y36kxiJwxKPKUrWAJbjs"),
    Coin(3500000000),
    std::vector<Output>{
        Output(Address::assertFromString("V7GghFKRA6BKqtHD7LTdT2ao93DRNA"),
               Coin(3499999999))},
    5904,
    publicationData,
    emptyBytes64,
    emptyBytes64};

static const std::string defaultTxEncoded =
    "014901011667a654ee3e0c918d8652b63829d7f3bef98524bf899604d09dc3000101166790"
    "1a1e11c650509efc46e09e81678054d8562af02b04d09dc2ff021710010801000100010001"
    "00400000000000000000000000000000000000000000000000000000000000000000000000"
    "00000000000000000000000000000000000000000000000000000000004000000000000000"
    "00000000000000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000";

TEST(VbkTx, DeserializeFromVbkEncoding) {
  auto decoded = AssertDeserializeFromHex<VbkTx>(defaultTxEncoded);

  EXPECT_EQ(decoded.networkOrType.typeId, defaultTx.networkOrType.typeId);
  EXPECT_EQ(decoded.sourceAddress, defaultTx.sourceAddress);
  EXPECT_EQ(decoded.sourceAmount, defaultTx.sourceAmount);
  EXPECT_EQ(decoded.signatureIndex, defaultTx.signatureIndex);
  EXPECT_EQ(decoded.outputs.size(), defaultTx.outputs.size());
  EXPECT_EQ(decoded.outputs[0], defaultTx.outputs[0]);
  EXPECT_EQ(decoded.publicationData.identifier,
            defaultTx.publicationData.identifier);
  EXPECT_EQ(decoded.signature, defaultTx.signature);
  EXPECT_EQ(decoded.publicKey, defaultTx.publicKey);
}

TEST(VbkTx, Serialize) {
  WriteStream stream;
  defaultTx.toVbkEncoding(stream);
  auto txBytes = stream.data();
  auto txEncoded = HexStr(txBytes);
  EXPECT_EQ(txEncoded, defaultTxEncoded);
}

TEST(VbkTx, RoundTrip) {
  auto decoded = AssertDeserializeFromHex<VbkTx>(defaultTxEncoded);
  EXPECT_EQ(decoded.signatureIndex, defaultTx.signatureIndex);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto txBytes = outputStream.data();
  auto txReEncoded = HexStr(txBytes);
  EXPECT_EQ(txReEncoded, defaultTxEncoded);
}

TEST(VbkTx, RoundTripNew) {
  auto txEncoded = ParseHex(defaultTxEncoded);
  VbkTx decoded;
  ValidationState state;
  bool ret = DeserializeFromVbkEncoding(txEncoded, decoded, state);
  ASSERT_TRUE(ret);
  EXPECT_TRUE(state.IsValid());
  EXPECT_EQ(decoded.signatureIndex, defaultTx.signatureIndex);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto txBytes = outputStream.data();
  auto txReEncoded = HexStr(txBytes);
  EXPECT_EQ(txReEncoded, defaultTxEncoded);
}
