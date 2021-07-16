// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

// clang-format off
#include <veriblock/pop/adaptors/picojson.hpp>
// clang-format on

#include "util/pop_test_fixture.hpp"
#include <veriblock/pop/literals.hpp>
#include <veriblock/pop/mock_miner.hpp>

using namespace altintegration;

void PrintTo(const VTB& v, std::ostream* os) {
  *os << ToJSON<picojson::value>(v).serialize(false);
}

TEST(ToJson, PopData) {
  PopData pd;
  pd.version = 1;
  pd.atvs.emplace_back();
  pd.vtbs.emplace_back();
  pd.context.emplace_back();

  auto json_0 = ToJSON<picojson::value>(pd, false).serialize(true);
  auto json_1 = ToJSON<picojson::value>(pd, true).serialize(true);

  std::string expected_0 = R"({
  "atvs": [
    "d1221f94e6a867306484dcc6d3bfbb69f41aa648f71a9e70da4a7d6750c96662"
  ],
  "vbkblocks": [
    "5534785fbe064bdebe45c01c"
  ],
  "version": 1,
  "vtbs": [
    "70fc5ecb12e2e31c39a888ff2eddf8a68ed4b9c5b623302fd584adaae842c22d"
  ]
}
)";

  std::string expected_1 = R"({
  "atvs": [
    {
      "blockOfProof": {
        "difficulty": 0,
        "hash": "afba108546bc1d7d173449155534785fbe064bdebe45c01c",
        "height": 0,
        "id": "5534785fbe064bdebe45c01c",
        "merkleRoot": "00000000000000000000000000000000",
        "nonce": 0,
        "previousBlock": "000000000000000000000000",
        "previousKeystone": "000000000000000000",
        "secondPreviousKeystone": "000000000000000000",
        "timestamp": 0,
        "version": 0
      },
      "id": "d1221f94e6a867306484dcc6d3bfbb69f41aa648f71a9e70da4a7d6750c96662",
      "merklePath": {
        "index": 0,
        "layers": [],
        "subject": "0000000000000000000000000000000000000000000000000000000000000000",
        "treeIndex": 0
      },
      "transaction": {
        "hash": "ff97657531892c9fa0541c61359eed115bb4ba0d1679b4e0a17046011c1acd14",
        "networkByte": null,
        "outputs": [],
        "publicKey": "",
        "publicationData": {
          "contextInfo": "",
          "header": "",
          "identifier": 0,
          "payoutInfo": ""
        },
        "signature": "",
        "signatureIndex": 0,
        "sourceAddress": "V111111111111111111111111G3LuZ",
        "sourceAmount": 0,
        "type": 0
      },
      "version": 1
    }
  ],
  "vbkblocks": [
    {
      "difficulty": 0,
      "hash": "afba108546bc1d7d173449155534785fbe064bdebe45c01c",
      "height": 0,
      "id": "5534785fbe064bdebe45c01c",
      "merkleRoot": "00000000000000000000000000000000",
      "nonce": 0,
      "previousBlock": "000000000000000000000000",
      "previousKeystone": "000000000000000000",
      "secondPreviousKeystone": "000000000000000000",
      "timestamp": 0,
      "version": 0
    }
  ],
  "version": 1,
  "vtbs": [
    {
      "containingBlock": {
        "difficulty": 0,
        "hash": "afba108546bc1d7d173449155534785fbe064bdebe45c01c",
        "height": 0,
        "id": "5534785fbe064bdebe45c01c",
        "merkleRoot": "00000000000000000000000000000000",
        "nonce": 0,
        "previousBlock": "000000000000000000000000",
        "previousKeystone": "000000000000000000",
        "secondPreviousKeystone": "000000000000000000",
        "timestamp": 0,
        "version": 0
      },
      "id": "70fc5ecb12e2e31c39a888ff2eddf8a68ed4b9c5b623302fd584adaae842c22d",
      "merklePath": {
        "index": 0,
        "layers": [],
        "subject": "0000000000000000000000000000000000000000000000000000000000000000",
        "treeIndex": 0
      },
      "transaction": {
        "address": "V111111111111111111111111G3LuZ",
        "bitcoinTransaction": "",
        "blockOfProof": {
          "bits": 0,
          "hash": "14508459b221041eab257d2baaa7459775ba748246c8403609eb708f0e57e74b",
          "merkleRoot": "0000000000000000000000000000000000000000000000000000000000000000",
          "nonce": 0,
          "previousBlock": "0000000000000000000000000000000000000000000000000000000000000000",
          "timestamp": 0,
          "version": 0
        },
        "blockOfProofContext": [],
        "hash": "9bfbefe12c9717b7f0ed1bee57b4e1100114a0e6836a2114dd7a7069a2fb0d85",
        "merklePath": {
          "index": 0,
          "layers": [],
          "subject": "0000000000000000000000000000000000000000000000000000000000000000"
        },
        "networkByte": null,
        "publicKey": "",
        "publishedBlock": {
          "difficulty": 0,
          "hash": "afba108546bc1d7d173449155534785fbe064bdebe45c01c",
          "height": 0,
          "id": "5534785fbe064bdebe45c01c",
          "merkleRoot": "00000000000000000000000000000000",
          "nonce": 0,
          "previousBlock": "000000000000000000000000",
          "previousKeystone": "000000000000000000",
          "secondPreviousKeystone": "000000000000000000",
          "timestamp": 0,
          "version": 0
        },
        "signature": "",
        "type": 0
      },
      "version": 1
    }
  ]
}
)";

  EXPECT_EQ(json_0, expected_0) << json_0;
  EXPECT_EQ(json_1, expected_1) << json_1;
}

TEST(ToJson, MempoolResult) {
  MempoolResult r;
  r.context.emplace_back();
  r.context.back().second.Invalid("bad-vbk", "Bad VBK!!!");
  r.atvs.emplace_back();
  r.vtbs.emplace_back();

  auto json = ToJSON<picojson::value>(r);
  auto actual = json.serialize(true);

  std::string expected = R"({
  "atvs": [
    {
      "id": "0000000000000000000000000000000000000000000000000000000000000000",
      "validity": {
        "state": "valid"
      }
    }
  ],
  "vbkblocks": [
    {
      "id": "000000000000000000000000",
      "validity": {
        "code": "bad-vbk",
        "message": "Bad VBK!!!",
        "state": "invalid"
      }
    }
  ],
  "vtbs": [
    {
      "id": "0000000000000000000000000000000000000000000000000000000000000000",
      "validity": {
        "state": "valid"
      }
    }
  ]
}
)";

  ASSERT_EQ(actual, expected);
}

TEST(ToJson, BtcBlock) {
  BtcBlock block;
  block.setVersion(1);
  block.setPreviousBlock(uint256::fromHex("123123123123123"));
  block.setMerkleRoot(uint256::fromHex("abcabc"));
  block.setDifficulty(1337);
  block.setTimestamp(9999);
  block.setNonce(9379992);

  picojson::value obj = ToJSON<picojson::value>(block);

  std::string expected = R"({
  "bits": 1337,
  "hash": "9025ce6e8008ae58912ebf1003ab80e8ca4f81f6c6dd065c12db47f93cd9722e",
  "merkleRoot": "abcabc0000000000000000000000000000000000000000000000000000000000",
  "nonce": 9379992,
  "previousBlock": "1231231231231200000000000000000000000000000000000000000000000000",
  "timestamp": 9999,
  "version": 1
}
)";

  auto actual = obj.serialize(true);

  ASSERT_EQ(actual, expected);
}

TEST(ToJson, VTB) {
  static const NetworkBytePair networkByte{{false, 0},
                                           (uint8_t)TxType::VBK_POP_TX};

  static const VbkBlock defaultVbkBlock{
      4917,
      2,
      "a793c872d6f6460e90bed623"_unhex,
      "42bb968195f8c515d3"_unhex,
      "eed7277a09efac4be9"_unhex,
      "9f95f0a15628b06ba3b44c0190b5c049"_unhex,
      1553697485,
      117556515,
      1589362332};

  static const auto btcTxBytes =
      "01000000010ce74f1fb694a001eebb1d7d08ce6208033f5bf7263ebad2de07bbf5186727"
      "32000000006a47304402200cf4998aba1682abeb777e762807a9dd2635a0b77773f66491"
      "b83ee3c87099ba022033b7ca24dc520915b8b0200cbdcf95ba6ae866354585af9c53ee86"
      "f27362ebec012103e5baf0709c395a82ef0bd63bc8847564ac201d69a8e6bf448d87aa53"
      "a1c431aaffffffff02b7270d00000000001976a9148b9ea8545059f3a922457afd14ddf3"
      "855d8b109988ac0000000000000000536a4c50000013350002a793c872d6f6460e90bed6"
      "2342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3b44c0190b5c0495c"
      "9b8acd0701c5235ebbbe9cd4e943efe1864df04216615cf92083f400000000"_unhex;

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
      "3045022100f4dce45edcc6bfc4a1f44ef04e47e90a348efd471f742f18b882ac77a8d0e8"
      "9e"
      "022061"
      "7cf7c4a22211991687b17126c1bb007a3b2a25c550f75d66b857a8fd9d75e7"_unhex;
  static const auto defaultPublicKey =
      "3056301006072a8648ce3d020106052b8104000a03420004b3c10470c8e8e426f1937758"
      "d9"
      "fb5e97"
      "a1891176cb37d4c12d4af4107b1aa3e8a8a754c06a22760e44c60642fba883967c19740d"
      "52"
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

  picojson::value val = ToJSON<picojson::value>(defaultVtb);
  auto actual = val.serialize(true);

  std::string expected = R"({
  "containingBlock": {
    "difficulty": 117576138,
    "hash": "9314b8e3d4ae11db85f006720f04294253c9f603e3ab6512",
    "height": 4976,
    "id": "0f04294253c9f603e3ab6512",
    "merkleRoot": "b53c1f4e259e6a0df23721a0b3b4b7ab",
    "nonce": -266584319,
    "previousBlock": "449c60619294546ad825af03",
    "previousKeystone": "b0935637860679ddd5",
    "secondPreviousKeystone": "5ee4fd21082e18686e",
    "timestamp": 1553699345,
    "version": 2
  },
  "id": "32dd01ab6285a92318e374fcdbd6b023c1ae1268150ce3484961870b8beb71fc",
  "merklePath": {
    "index": 13,
    "layers": [
      "5b977ea09a554ad56957f662284044e7d37450ddadf7db3647712f5969399787",
      "20d0a3d873eeeee6a222a75316dce60b53ca43eaea09d27f0ece897303a53ae9",
      "c06fe913dca5dc2736563b80834d69e6dfdf1b1e92383ea62791e410421b6c11",
      "049f68d350eeb8b3df630c8308b5c8c2ba4cd6210868395b084af84d19ff0e90",
      "0000000000000000000000000000000000000000000000000000000000000000",
      "36252dfc621de420fb083ad9d8767cba627eddeec64e421e9576cee21297dd0a"
    ],
    "subject": "2a014e88ed7ab65cdfaa85daeab07eea6cba5e147f736edd8d02c2f9ddf0dec6",
    "treeIndex": 0
  },
  "transaction": {
    "address": "VE6MJFzmGdYdrxC8o6UCovVv7BdhdX",
    "bitcoinTransaction": "01000000010ce74f1fb694a001eebb1d7d08ce6208033f5bf7263ebad2de07bbf518672732000000006a47304402200cf4998aba1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87099ba022033b7ca24dc520915b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103e5baf0709c395a82ef0bd63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270d00000000001976a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac0000000000000000536a4c50000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe1864df04216615cf92083f400000000",
    "blockOfProof": {
      "bits": 388767596,
      "hash": "08432b58e00c66a96e1a69a063fb9d32b0f66239de7e1a78061f99cba88de7f8",
      "merkleRoot": "ce0b1a9a77dd0db127b5df4bc368cd6ac299a9747d991ec2dacbc0b699a2e4a5",
      "nonce": 3225120883,
      "previousBlock": "134f09d43659eb53982d9afb444b96fa4bb58c037d2914000000000000000000",
      "timestamp": 1553699251,
      "version": 549453824
    },
    "blockOfProofContext": [
      {
        "bits": 388767596,
        "hash": "6170e8d73507ca1fddb4ef10d22f6d43360658c68d35b664a968d5788097d0ed",
        "merkleRoot": "a31508d4b101d0ad11e43ef9419c23fc277f67edae83c598ee70866dbcef5e25",
        "nonce": 2943621345,
        "previousBlock": "fc61cc9d4eac4b2d14761a4d06af8a9ef073dcd7fb5e0d000000000000000000",
        "timestamp": 1553697574,
        "version": 545259520
      },
      {
        "bits": 388767596,
        "hash": "c11dcd341d3815fa39d30bc70734b2d4254ce48bb49a8fa2711409bfaa3e957a",
        "merkleRoot": "39a72c22268381bd8d9dcfe002f472634a24cf0454de8b50f89e10891e5ffb1d",
        "nonce": 2450139460,
        "previousBlock": "3f8e3980304439d853c302f6e496285e110e2512515313000000000000000000",
        "timestamp": 1553698272,
        "version": 541065216
      },
      {
        "bits": 388767596,
        "hash": "a84f0e968c19379abb479787fc282f85bef51d6863b5ad421adca7770b894853",
        "merkleRoot": "a33ad6be0634647b26633ab85fa8de258480bbb25e59c68e48bb0b608b12362b",
        "nonce": 4040279113,
        "previousBlock": "baa42e40345a7f826a31d37db1a5d64b67b72732477422000000000000000000",
        "timestamp": 1553699088,
        "version": 536870912
      }
    ],
    "hash": "195a793a5fb94a749876c71a40bd55d705184fd613b908564070c2c6bc553277",
    "merklePath": {
      "index": 1659,
      "layers": [
        "4d66077fdf24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f612",
        "023d1abe8758c6f917ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af1",
        "096ddba03ca952af133fb06307c24171e53bf50ab76f1edeabde5e99f78d4ead",
        "2f32cf1bee50349d56fc1943af84f2d2abda520f64dc4db37b2f3db20b0ecb57",
        "93e70120f1b539d0c1495b368061129f30d35f9e436f32d69967ae86031a2756",
        "f554378a116e2142f9f6315a38b19bd8a1b2e6dc31201f2d37a058f03c39c06c",
        "0824705685ceca003c95140434ee9d8bbbf4474b83fd4ecc2766137db9a44d74",
        "b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a74453fb162",
        "1732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad17",
        "2d9b57e92ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb99",
        "dcba229acdc6b7f028ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618",
        "2cf1439a6dbcc1a35e96574bddbf2c5db9174af5ad0d278fe92e06e4ac349a42"
      ],
      "subject": "94e097b110ba3adbb7b6c4c599d31d675de7be6e722407410c08ef352be585f1"
    },
    "networkByte": null,
    "publicKey": "3056301006072a8648ce3d020106052b8104000a03420004b3c10470c8e8e426f1937758d9fb5e97a1891176cb37d4c12d4af4107b1aa3e8a8a754c06a22760e44c60642fba883967c19740d5231336326f7962750c8df99",
    "publishedBlock": {
      "difficulty": 117556515,
      "hash": "e5fcf4d45317a7de93a03fb12e785abd956fadd2bff6336b",
      "height": 4917,
      "id": "2e785abd956fadd2bff6336b",
      "merkleRoot": "9f95f0a15628b06ba3b44c0190b5c049",
      "nonce": 1589362332,
      "previousBlock": "a793c872d6f6460e90bed623",
      "previousKeystone": "42bb968195f8c515d3",
      "secondPreviousKeystone": "eed7277a09efac4be9",
      "timestamp": 1553697485,
      "version": 2
    },
    "signature": "3045022100f4dce45edcc6bfc4a1f44ef04e47e90a348efd471f742f18b882ac77a8d0e89e0220617cf7c4a22211991687b17126c1bb007a3b2a25c550f75d66b857a8fd9d75e7",
    "type": 2
  },
  "version": 1
}
)";

  ASSERT_EQ(expected, actual) << actual;
}

TEST(ToJson, ATV) {
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

  static const VbkBlock defaultVbkBlock{
      5000,
      2,
      "449c60619294546ad825af03"_unhex,
      "b0935637860679ddd5"_unhex,
      "5ee4fd21082e18686e"_unhex,
      "26bbfda7d5e4462ef24ae02d67e47d78"_unhex,
      1553699059,
      16842752,
      1};

  static const ATV defaultAtv{1, defaultTx, defaultPath, defaultVbkBlock};

  picojson::value val = ToJSON<picojson::value>(defaultAtv);
  auto actual = val.serialize(true);

  auto expected = R"({
  "blockOfProof": {
    "difficulty": 16842752,
    "hash": "2ba076219b4ff7ed36512275cd97599e23096ad42f119b5a",
    "height": 5000,
    "id": "cd97599e23096ad42f119b5a",
    "merkleRoot": "26bbfda7d5e4462ef24ae02d67e47d78",
    "nonce": 1,
    "previousBlock": "449c60619294546ad825af03",
    "previousKeystone": "b0935637860679ddd5",
    "secondPreviousKeystone": "5ee4fd21082e18686e",
    "timestamp": 1553699059,
    "version": 2
  },
  "id": "c6d96b8e87f3e347aa1d1051bb3af39c8ea60612ced905d11c6f92d7b6bd50f5",
  "merklePath": {
    "index": 0,
    "layers": [
      "0000000000000000000000000000000000000000000000000000000000000000",
      "0000000000000000000000000000000000000000000000000000000000000000"
    ],
    "subject": "1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c",
    "treeIndex": 1
  },
  "transaction": {
    "hash": "1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c",
    "networkByte": null,
    "outputs": [],
    "publicKey": "3056301006072a8648ce3d020106052b8104000a03420004de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e",
    "publicationData": {
      "contextInfo": "636f6e7465787420696e666f206279746573",
      "header": "686561646572206279746573",
      "identifier": 0,
      "payoutInfo": "7061796f757420696e666f206279746573"
    },
    "signature": "30440220398b74708dc8f8aee68fce0c47b8959e6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d8df2b2117e75acbb9db7deb3c7",
    "signatureIndex": 7,
    "sourceAddress": "V5Ujv72h4jEBcKnALGc4fKqs6CDAPX",
    "sourceAmount": 1000,
    "type": 1
  },
  "version": 1
}
)";

  ASSERT_EQ(expected, actual) << actual;
}

TEST(ToJson, VbkBlockIndex) {
  MockMiner m;
  picojson::value block =
      ToJSON<picojson::value>(*m.vbk().getBestChain().tip());
  std::string actual = block.serialize(true);

  std::string expected = R"({
  "altrefs": 1,
  "blockOfProofEndorsements": [],
  "chainWork": "0000000000000000000000000000000000000000000000000000000000000001",
  "containingEndorsements": [],
  "endorsedBy": [],
  "header": {
    "difficulty": 16842752,
    "hash": "a3e77a18a8f7b4568a062f69340d1ad4360382e5bc218a8c",
    "height": 0,
    "id": "340d1ad4360382e5bc218a8c",
    "merkleRoot": "80d7178046d25ca9ad283c5af587a7c5",
    "nonce": 0,
    "previousBlock": "000000000000000000000000",
    "previousKeystone": "000000000000000000",
    "secondPreviousKeystone": "000000000000000000",
    "timestamp": 1603044490,
    "version": 2
  },
  "height": 0,
  "status": 532,
  "stored": {
    "vtbids": []
  }
}
)";

  ASSERT_EQ(expected, actual) << actual;
}

TEST(ToJson, BtcBlockIndex) {
  MockMiner m;
  auto* index = m.btc().getBestChain().tip();
  picojson::value block = ToJSON<picojson::value>(*index);
  std::string actual = block.serialize(true);

  std::string expected = R"({
  "blockOfProofEndorsements": [],
  "chainWork": "0000000000000000000000000000000000000000000000000000000000000002",
  "header": {
    "bits": 545259519,
    "hash": "0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206",
    "merkleRoot": "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b",
    "nonce": 2,
    "previousBlock": "0000000000000000000000000000000000000000000000000000000000000000",
    "timestamp": 1296688602,
    "version": 1
  },
  "height": 0,
  "status": 532,
  "vbkrefs": [
    0
  ]
}
)";

  ASSERT_EQ(expected, actual) << actual;
}

TEST(ToJson, AltParams) {
  std::shared_ptr<AltChainParams> rt =
      std::make_shared<AltChainParamsRegTest>();
  rt->mKeystoneInterval = 5;
  auto json = ToJSON<picojson::value>(*rt);
  std::string actual = json.serialize(true);

  std::string expected = R"({
  "bootstrapBlock": {
    "hash": "010101010101010101010101",
    "height": 0,
    "previousBlock": "000000000000000000000000",
    "timestamp": 0
  },
  "endorsementSettlementInterval": 50,
  "finalityDelay": 100,
  "forkResolutionLookupTable": [
    100,
    100,
    95,
    89,
    80,
    69,
    56,
    40,
    21
  ],
  "keystoneInterval": 5,
  "maxATVsInAltBlock": 1000,
  "maxAltchainFutureBlockTime": 600,
  "maxReorgDistance": 50000,
  "maxVTBsInAltBlock": 200,
  "maxVbkBlocksInAltBlock": 200,
  "networkId": 0,
  "payoutParams": {
    "difficultyAveragingInterval": 50,
    "flatScoreRound": 2,
    "keystoneRound": 3,
    "lookupTable": [
      1,
      1,
      1,
      1,
      1,
      1,
      1,
      1,
      1,
      1,
      1,
      1,
      0.48296815999999998,
      0.31551694000000002,
      0.23325824000000001,
      0.18453616,
      0.15238462999999999,
      0.12961254999999999,
      0.1126563,
      0.099550940000000004,
      0.089125090000000004,
      0.080637609999999998,
      0.073596919999999996,
      0.067664279999999993,
      0.062598730000000005,
      0.058224280000000003,
      0.054409409999999998,
      0.05105386,
      0.04807993,
      0.045426439999999998,
      0.043044579999999999,
      0.040894949999999999,
      0.038945399999999998,
      0.03716941,
      0.035544970000000002,
      0.034053590000000002,
      0.032679689999999997,
      0.03141,
      0.03023319,
      0.029139499999999999,
      0.028120470000000002,
      0.02716878,
      0.026278010000000001,
      0.025442530000000001,
      0.024657390000000001,
      0.023918200000000001,
      0.02322107,
      0.022562550000000001,
      0.021939520000000001,
      0.021349219999999999
    ],
    "maxScoreThresholdKeystone": 3,
    "maxScoreThresholdNormal": 2,
    "payoutRounds": 4,
    "popPayoutDelay": 50,
    "roundRatios": [
      0.96999999999999997,
      1.03,
      1.0700000000000001,
      3
    ],
    "slopeKeystone": 0.21325,
    "slopeNormal": 0.20000000000000001,
    "startOfSlope": 1,
    "useFlatScoreRound": true
  }
}
)";

  ASSERT_EQ(expected, actual) << actual;
}