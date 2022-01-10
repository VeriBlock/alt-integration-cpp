// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/vbkpoptx.hpp>

#include <gtest/gtest.h>

#include <veriblock/pop/literals.hpp>

using namespace altintegration;

static const NetworkBytePair networkByte{{false, 0},
                                         (uint8_t)TxType::VBK_POP_TX};

static const VbkBlock defaultVbkBlock{4917,
                                      2,
                                      "a793c872d6f6460e90bed623"_unhex,
                                      "42bb968195f8c515d3"_unhex,
                                      "eed7277a09efac4be9"_unhex,
                                      "9f95f0a15628b06ba3b44c0190b5c049"_unhex,
                                      1553697485,
                                      117556515,
                                      1589362332};

static const auto btcTxBytes =
    "01000000010ce74f1fb694a001eebb1d7d08ce6208033f5bf7263ebad2de07bbf518672732"
    "00000"
    "0006a47304402200cf4998aba1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87"
    "099ba"
    "022033b7ca24dc520915b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103"
    "e5baf"
    "0709c395a82ef0bd63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270"
    "d0000"
    "0000001976a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac0000000000000000"
    "536a4"
    "c50000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be"
    "99f95"
    "f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe1864df0421661"
    "5cf92"
    "083f400000000"_unhex;

static const MerklePath defaultPath{
    1659,
    uint256(
        "94E097B110BA3ADBB7B6C4C599D31D675DE7BE6E722407410C08EF352BE585F1"_unhex),
    std::vector<uint256>{
        uint256(
            "4D66077FDF24246FFD6B6979DFEDEF5D46588654ADDEB35EDB11E993C131F612"_unhex),
        uint256(
            "023D1ABE8758C6F917EC0C65674BBD43D66EE14DC667B3117DFC44690C6F5AF1"_unhex),
        uint256(
            "096DDBA03CA952AF133FB06307C24171E53BF50AB76F1EDEABDE5E99F78D4EAD"_unhex),
        uint256(
            "2F32CF1BEE50349D56FC1943AF84F2D2ABDA520F64DC4DB37B2F3DB20B0ECB57"_unhex),
        uint256(
            "93E70120F1B539D0C1495B368061129F30D35F9E436F32D69967AE86031A2756"_unhex),
        uint256(
            "F554378A116E2142F9F6315A38B19BD8A1B2E6DC31201F2D37A058F03C39C06C"_unhex),
        uint256(
            "0824705685CECA003C95140434EE9D8BBBF4474B83FD4ECC2766137DB9A44D74"_unhex),
        uint256(
            "B7B9E52F3EE8CE4FBB8BE7D6CF66D33A20293F806C69385136662A74453FB162"_unhex),
        uint256(
            "1732C9A35E80D4796BABEA76AACE50B49F6079EA3E349F026B4491CFE720AD17"_unhex),
        uint256(
            "2D9B57E92AB51FE28A587050FD82ABB30ABD699A5CE8B54E7CD49B2A827BCB99"_unhex),
        uint256(
            "DCBA229ACDC6B7F028BA756FD5ABBFEBD31B4227CD4137D728EC5EA56C457618"_unhex),
        uint256(
            "2CF1439A6DBCC1A35E96574BDDBF2C5DB9174AF5AD0D278FE92E06E4AC349A42"_unhex)}};

static const BtcBlock defaultBtcBlock{
    549453824,
    uint256(
        "134f09d43659eb53982d9afb444b96fa4bb58c037d2914000000000000000000"_unhex),
    uint256(
        "ce0b1a9a77dd0db127b5df4bc368cd6ac299a9747d991ec2dacbc0b699a2e4a5"_unhex),
    1553699251,
    388767596,
    (uint32_t)-1069846413};

static const BtcBlock btcBlock1{
    545259520,
    uint256(
        "fc61cc9d4eac4b2d14761a4d06af8a9ef073dcd7fb5e0d000000000000000000"_unhex),
    uint256(
        "a31508d4b101d0ad11e43ef9419c23fc277f67edae83c598ee70866dbcef5e25"_unhex),
    1553697574,
    388767596,
    (uint32_t)-1351345951};

static const BtcBlock btcBlock2{
    541065216,
    uint256(
        "3f8e3980304439d853c302f6e496285e110e2512515313000000000000000000"_unhex),
    uint256(
        "39a72c22268381bd8d9dcfe002f472634a24cf0454de8b50f89e10891e5ffb1d"_unhex),
    1553698272,
    388767596,
    (uint32_t)-1844827836};

static const BtcBlock btcBlock3{
    536870912,
    uint256(
        "baa42e40345a7f826a31d37db1a5d64b67b72732477422000000000000000000"_unhex),
    uint256(
        "a33ad6be0634647b26633ab85fa8de258480bbb25e59c68e48bb0b608b12362b"_unhex),
    1553699088,
    388767596,
    (uint32_t)-254688183};

static const auto defaultSignature =
    "3045022100f4dce45edcc6bfc4a1f44ef04e47e90a348efd471f742f18b882ac77a8d0e89e"
    "022061"
    "7cf7c4a22211991687b17126c1bb007a3b2a25c550f75d66b857a8fd9d75e7"_unhex;
static const auto defaultPublicKey =
    "3056301006072a8648ce3d020106052b8104000a03420004b3c10470c8e8e426f1937758d9"
    "fb5e97"
    "a1891176cb37d4c12d4af4107b1aa3e8a8a754c06a22760e44c60642fba883967c19740d52"
    "313363"
    "26f7962750c8df99"_unhex;

static const VbkPopTx defaultTx{
    networkByte,
    Address::assertFromString("VE6MJFzmGdYdrxC8o6UCovVv7BdhdX"),
    defaultVbkBlock,
    BtcTx(btcTxBytes),
    defaultPath,
    defaultBtcBlock,
    std::vector<BtcBlock>{btcBlock1, btcBlock2, btcBlock3},
    defaultSignature,
    defaultPublicKey};

static const std::string defaultTxEncoded =
    "02046102011667ff0a897e5d512a0b6da2f41c479867fe6b3a4cae2641000013350002a793"
    "c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3"
    "b44c0190b5c0495c9b8acd0701c523005ebbbe9c02011b01000000010ce74f1fb694a001ee"
    "bb1d7d08ce6208033f5bf7263ebad2de07bbf518672732000000006a47304402200cf4998a"
    "ba1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87099ba022033b7ca24dc5209"
    "15b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103e5baf0709c395a82ef"
    "0bd63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270d000000000019"
    "76a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac0000000000000000536a4c50"
    "000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f"
    "95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe1864df04216"
    "615cf92083f40000000002019f040000067b040000000c040000000400000020204d66077f"
    "df24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f61220023d1abe8758c6f9"
    "17ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af120096ddba03ca952af133fb063"
    "07c24171e53bf50ab76f1edeabde5e99f78d4ead202f32cf1bee50349d56fc1943af84f2d2"
    "abda520f64dc4db37b2f3db20b0ecb572093e70120f1b539d0c1495b368061129f30d35f9e"
    "436f32d69967ae86031a275620f554378a116e2142f9f6315a38b19bd8a1b2e6dc31201f2d"
    "37a058f03c39c06c200824705685ceca003c95140434ee9d8bbbf4474b83fd4ecc2766137d"
    "b9a44d7420b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a74453fb162"
    "201732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad17202d9b57"
    "e92ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb9920dcba229acdc6b7"
    "f028ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618202cf1439a6dbcc1a35e9657"
    "4bddbf2c5db9174af5ad0d278fe92e06e4ac349a42500000c0200000000000000000001429"
    "7d038cb54bfa964b44fb9a2d9853eb5936d4094f13a5e4a299b6c0cbdac21e997d74a999c2"
    "6acd68c34bdfb527b10ddd779a1a0bceb3919b5c6c1f2c1773703bc0010350000080200000"
    "000000000000000d5efbd7dc73f09e8aaf064d1a76142d4bac4e9dcc61fc255eefbc6d8670"
    "ee98c583aeed677f27fc239c41f93ee411add001b1d40815a3268b9b5c6c1f2c17e11874af"
    "500000402000000000000000000013535112250e115e2896e4f602c353d839443080398e3f"
    "1dfb5f1e89109ef8508bde5404cf244a6372f402e0cf9d8dbd818326222ca739e08d9b5c6c"
    "1f2c1744290a9250000000200000000000000000002274473227b7674bd6a5b17dd3316a82"
    "7f5a34402ea4ba2b36128b600bbb488ec6595eb2bb808425dea85fb83a63267b643406bed6"
    "3aa310919b5c6c1f2c1749c4d1f0473045022100f4dce45edcc6bfc4a1f44ef04e47e90a34"
    "8efd471f742f18b882ac77a8d0e89e0220617cf7c4a22211991687b17126c1bb007a3b2a25"
    "c550f75d66b857a8fd9d75e7583056301006072a8648ce3d020106052b8104000a03420004"
    "b3c10470c8e8e426f1937758d9fb5e97a1891176cb37d4c12d4af4107b1aa3e8a8a754c06a"
    "22760e44c60642fba883967c19740d5231336326f7962750c8df99";

TEST(VbkPopTx, DeserializeFromVbkEncoding) {
  auto decoded = AssertDeserializeFromHex<VbkPopTx>(defaultTxEncoded);
  EXPECT_EQ(decoded.networkOrType.typeId, defaultTx.networkOrType.typeId);
  EXPECT_EQ(decoded.address, defaultTx.address);
  EXPECT_EQ(decoded.publishedBlock.getHeight(),
            defaultTx.publishedBlock.getHeight());
  EXPECT_EQ(decoded.bitcoinTransaction.tx, defaultTx.bitcoinTransaction.tx);
  EXPECT_EQ(decoded.blockOfProof.getVersion(),
            defaultTx.blockOfProof.getVersion());
  EXPECT_EQ(decoded.blockOfProofContext.size(),
            defaultTx.blockOfProofContext.size());
  EXPECT_EQ(decoded.blockOfProofContext[0].getVersion(),
            defaultTx.blockOfProofContext[0].getVersion());
  EXPECT_EQ(decoded.merklePath.index, defaultTx.merklePath.index);
  EXPECT_EQ(decoded.signature, defaultTx.signature);
  EXPECT_EQ(decoded.publicKey, defaultTx.publicKey);
}

TEST(VbkPopTx, Serialize) {
  WriteStream stream;
  defaultTx.toVbkEncoding(stream);
  auto txBytes = stream.data();
  auto txEncoded = HexStr(txBytes);
  EXPECT_EQ(txEncoded, defaultTxEncoded);
}

TEST(VbkPopTx, RoundTrip) {
  auto decoded = AssertDeserializeFromHex<VbkPopTx>(defaultTxEncoded);
  EXPECT_EQ(decoded.address,
            Address::assertFromString("VE6MJFzmGdYdrxC8o6UCovVv7BdhdX"));

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto txBytes = outputStream.data();
  auto txReEncoded = HexStr(txBytes);
  EXPECT_EQ(txReEncoded, defaultTxEncoded);
}

TEST(VbkPopTx, RoundTripNew) {
  auto txEncoded = ParseHex(defaultTxEncoded);
  VbkPopTx decoded;
  ValidationState state;
  bool ret = DeserializeFromVbkEncoding(txEncoded, decoded, state);
  ASSERT_TRUE(ret);
  EXPECT_TRUE(state.IsValid());
  EXPECT_EQ(decoded.address,
            Address::assertFromString("VE6MJFzmGdYdrxC8o6UCovVv7BdhdX"));

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto txBytes = outputStream.data();
  auto txReEncoded = HexStr(txBytes);
  EXPECT_EQ(txReEncoded, defaultTxEncoded);
}
