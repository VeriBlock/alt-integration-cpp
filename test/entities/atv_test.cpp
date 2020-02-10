#include <gtest/gtest.h>

#include "util/literals.hpp"
#include "veriblock/entities/atv.hpp"

using namespace VeriBlock;

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
    NetworkBytePair{false, 0, (uint8_t)TxType::VBK_TX},
    Address::fromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"),
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

static const ATV defaultAtv{
    defaultTx, defaultPath, defaultVbkBlock, std::vector<VbkBlock>{}};

static const std::string defaultAtvEncoded =
    "01580101166772f51ab208d32771ab1506970eeb664462730b838e0203e800010701370100"
    "010c6865616465722062797465730112636f6e7465787420696e666f206279746573011170"
    "61796f757420696e666f2062797465734630440220398b74708dc8f8aee68fce0c47b8959e"
    "6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d"
    "8df2b2117e75acbb9db7deb3c7583056301006072a8648ce3d020106052b8104000a034200"
    "04de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692"
    "b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e040000000104000000"
    "00201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c040000"
    "00022000000000000000000000000000000000000000000000000000000000000000002000"
    "00000000000000000000000000000000000000000000000000000000000000400000138800"
    "02449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4"
    "462ef24ae02d67e47d785c9b90f301010000000000010100";

TEST(ATV, Deserialize) {
  const auto atvBytes = ParseHex(defaultAtvEncoded);
  auto stream = ReadStream(atvBytes);
  auto decoded = ATV::fromVbkEncoding(stream);

  EXPECT_EQ(decoded.transaction.sourceAddress,
            Address::fromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"));
}

TEST(ATV, Serialize) {
  WriteStream stream;
  defaultAtv.toVbkEncoding(stream);
  auto atvBytes = stream.data();
  auto atvEncoded = HexStr(atvBytes);
  EXPECT_EQ(atvEncoded, defaultAtvEncoded);
}

TEST(ATV, RoundTrip) {
  auto atvBytes = ParseHex(defaultAtvEncoded);
  auto stream = ReadStream(atvBytes);
  auto decoded = ATV::fromVbkEncoding(stream);
  EXPECT_EQ(decoded.transaction.sourceAddress,
            Address::fromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"));

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto txBytes = outputStream.data();
  auto txReEncoded = HexStr(txBytes);
  EXPECT_EQ(txReEncoded, defaultAtvEncoded);
}
