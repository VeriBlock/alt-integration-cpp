// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <veriblock/pop/entities/vtb.hpp>
#include <veriblock/pop/literals.hpp>

#include "pop/util/test_utils.hpp"

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

static const VbkMerklePath vtbProofPath{
    0,
    13,
    uint256(
        "2a014e88ed7ab65cdfaa85daeab07eea6cba5e147f736edd8d02c2f9ddf0dec6"_unhex),
    std::vector<uint256>{
        uint256(
            "5b977ea09a554ad56957f662284044e7d37450ddadf7db3647712f5969399787"_unhex),
        uint256(
            "20d0a3d873eeeee6a222a75316dce60b53ca43eaea09d27f0ece897303a53ae9"_unhex),
        uint256(
            "c06fe913dca5dc2736563b80834d69e6dfdf1b1e92383ea62791e410421b6c11"_unhex),
        uint256(
            "049f68d350eeb8b3df630c8308b5c8c2ba4cd6210868395b084af84d19ff0e90"_unhex),
        uint256(
            "0000000000000000000000000000000000000000000000000000000000000000"_unhex),
        uint256(
            "36252dfc621de420fb083ad9d8767cba627eddeec64e421e9576cee21297dd0a"_unhex)}};

static const VbkBlock vtbVbkBlock{4976,
                                  2,
                                  "449c60619294546ad825af03"_unhex,
                                  "b0935637860679ddd5"_unhex,
                                  "5ee4fd21082e18686e"_unhex,
                                  "b53c1f4e259e6a0df23721a0b3b4b7ab"_unhex,
                                  1553699345,
                                  117576138,
                                  static_cast<uint64_t>(-266584319)};

static const VTB defaultVtb{1, defaultTx, vtbProofPath, vtbVbkBlock};

TEST(VTB, DeserializeFromVbkEncoding) {
  auto decoded = AssertDeserializeFromHex<VTB>(defaultVtbEncoded);

  EXPECT_EQ(decoded.transaction.address,
            Address::assertFromString("VE6MJFzmGdYdrxC8o6UCovVv7BdhdX"));
}

TEST(VTB, Serialize) {
  WriteStream stream;
  defaultVtb.toVbkEncoding(stream);
  auto vtbBytes = stream.data();
  auto vtbEncoded = HexStr(vtbBytes);
  EXPECT_EQ(vtbEncoded, defaultVtbEncoded);
}

TEST(VTB, RoundTrip) {
  auto decoded = AssertDeserializeFromHex<VTB>(defaultVtbEncoded);
  EXPECT_EQ(decoded.transaction.address,
            Address::assertFromString("VE6MJFzmGdYdrxC8o6UCovVv7BdhdX"));

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto txBytes = outputStream.data();
  auto txReEncoded = HexStr(txBytes);
  EXPECT_EQ(txReEncoded, defaultVtbEncoded);
}

TEST(VTB, RoundTripNew) {
  auto vtbBytes = ParseHex(defaultVtbEncoded);
  VTB decoded;
  ValidationState state;
  bool ret = DeserializeFromVbkEncoding(vtbBytes, decoded, state);
  ASSERT_TRUE(ret);
  EXPECT_TRUE(state.IsValid());
  EXPECT_EQ(decoded.transaction.address,
            Address::assertFromString("VE6MJFzmGdYdrxC8o6UCovVv7BdhdX"));

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto txBytes = outputStream.data();
  auto txReEncoded = HexStr(txBytes);
  EXPECT_EQ(txReEncoded, defaultVtbEncoded);
}

TEST(VTB, getId_test) {
  auto vtb = AssertDeserializeFromHex<VTB>(defaultVtbEncoded);
  EXPECT_EQ(vtb.getId().toHex(),
            "32dd01ab6285a92318e374fcdbd6b023c1ae1268150ce3484961870b8beb71fc");
}
