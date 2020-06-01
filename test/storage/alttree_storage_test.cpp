#include <gtest/gtest.h>

#include "util/literals.hpp"
#include <util/alt_chain_params_regtest.hpp>
#include <veriblock/entities/payloads.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/popdata.hpp>
#include <veriblock/storage/pop_storage.hpp>
#include <veriblock/storage/endorsement_storage.hpp>
#include <veriblock/storage/block_repository_inmem.hpp>

using namespace altintegration;

static const std::vector<uint8_t> defaultVtbEncoded = ParseHex(
    "02046002011667ff0a897e5d512a0b6da2f41c479867fe6b3a4cae2640000013350002a793"
    "c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3"
    "b44c0190b5c0495c9b8acd0701c5235ebbbe9c02011b01000000010ce74f1fb694a001eebb"
    "1d7d08ce6208033f5bf7263ebad2de07bbf518672732000000006a47304402200cf4998aba"
    "1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87099ba022033b7ca24dc520915"
    "b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103e5baf0709c395a82ef0b"
    "d63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270d00000000001976"
    "a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac0000000000000000536a4c5000"
    "0013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95"
    "f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe1864df0421661"
    "5cf92083f40000000002019f040000067b040000000c040000000400000020204d66077fdf"
    "24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f61220023d1abe8758c6f917"
    "ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af120096ddba03ca952af133fb06307"
    "c24171e53bf50ab76f1edeabde5e99f78d4ead202f32cf1bee50349d56fc1943af84f2d2ab"
    "da520f64dc4db37b2f3db20b0ecb572093e70120f1b539d0c1495b368061129f30d35f9e43"
    "6f32d69967ae86031a275620f554378a116e2142f9f6315a38b19bd8a1b2e6dc31201f2d37"
    "a058f03c39c06c200824705685ceca003c95140434ee9d8bbbf4474b83fd4ecc2766137db9"
    "a44d7420b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a74453fb16220"
    "1732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad17202d9b57e9"
    "2ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb9920dcba229acdc6b7f0"
    "28ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618202cf1439a6dbcc1a35e96574b"
    "ddbf2c5db9174af5ad0d278fe92e06e4ac349a42500000c02000000000000000000014297d"
    "038cb54bfa964b44fb9a2d9853eb5936d4094f13a5e4a299b6c0cbdac21e997d74a999c26a"
    "cd68c34bdfb527b10ddd779a1a0bceb3919b5c6c1f2c1773703bc001035000008020000000"
    "0000000000000d5efbd7dc73f09e8aaf064d1a76142d4bac4e9dcc61fc255eefbc6d8670ee"
    "98c583aeed677f27fc239c41f93ee411add001b1d40815a3268b9b5c6c1f2c17e11874af50"
    "0000402000000000000000000013535112250e115e2896e4f602c353d839443080398e3f1d"
    "fb5f1e89109ef8508bde5404cf244a6372f402e0cf9d8dbd818326222ca739e08d9b5c6c1f"
    "2c1744290a9250000000200000000000000000002274473227b7674bd6a5b17dd3316a827f"
    "5a34402ea4ba2b36128b600bbb488ec6595eb2bb808425dea85fb83a63267b643406bed63a"
    "a310919b5c6c1f2c1749c4d1f0473045022100f4dce45edcc6bfc4a1f44ef04e47e90a348e"
    "fd471f742f18b882ac77a8d0e89e0220617cf7c4a22211991687b17126c1bb007a3b2a25c5"
    "50f75d66b857a8fd9d75e7583056301006072a8648ce3d020106052b8104000a03420004b3"
    "c10470c8e8e426f1937758d9fb5e97a1891176cb37d4c12d4af4107b1aa3e8a8a754c06a22"
    "760e44c60642fba883967c19740d5231336326f7962750c8df990400000000040000000d20"
    "2a014e88ed7ab65cdfaa85daeab07eea6cba5e147f736edd8d02c2f9ddf0dec60400000006"
    "205b977ea09a554ad56957f662284044e7d37450ddadf7db3647712f59693997872020d0a3"
    "d873eeeee6a222a75316dce60b53ca43eaea09d27f0ece897303a53ae920c06fe913dca5dc"
    "2736563b80834d69e6dfdf1b1e92383ea62791e410421b6c1120049f68d350eeb8b3df630c"
    "8308b5c8c2ba4cd6210868395b084af84d19ff0e9020000000000000000000000000000000"
    "00000000000000000000000000000000002036252dfc621de420fb083ad9d8767cba627edd"
    "eec64e421e9576cee21297dd0a40000013700002449c60619294546ad825af03b093563786"
    "0679ddd55ee4fd21082e18686eb53c1f4e259e6a0df23721a0b3b4b7ab5c9b9211070211ca"
    "f01c3f010100");

static const std::vector<uint8_t> defaultAtvEncoded = ParseHex(
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
    "462ef24ae02d67e47d785c9b90f301010000000000010100");

struct AltTreeRepositoryTest : public ::testing::Test {
  std::shared_ptr<PayloadsRepository<AltPayloads>> payloadsRepo;

  AltTreeRepositoryTest() {
    payloadsRepo = std::make_shared<PayloadsRepositoryInmem<AltPayloads>>();
  }
};

AltPayloads getDefaultContainer() {
  auto endorsed = AltBlock{{1, 2, 3}, {}, 123, 4125};
  auto containingBlock = AltBlock{{11, 12, 24}, {}, 1247, 1425};
  auto popData = PopData{};
  popData.atv = ATV::fromVbkEncoding(defaultAtvEncoded);
  popData.hasAtv = true;
  auto payloads = AltPayloads{endorsed, containingBlock, popData};
  return payloads;
}

AltPayloads getModifiedContainer() {
  auto endorsed = AltBlock{{1, 2, 3}, {}, 123, 4125};
  auto containingBlock = AltBlock{{11, 12, 24}, {}, 1247, 1425};
  auto popData = PopData{};
  popData.atv = ATV::fromVbkEncoding(defaultAtvEncoded);
  popData.atv.transaction.sourceAmount = Coin(213);
  popData.hasAtv = true;
  auto payloads = AltPayloads{endorsed, containingBlock, popData};
  return payloads;
}

TEST_F(AltTreeRepositoryTest, Basic) {
  PopStorage storage{};
  BtcChainParamsRegTest btcparam{};
  VbkChainParamsRegTest vbkparam{};
  AltChainParamsRegTest altparam{};

  // trees
  AltTree alttree = AltTree(altparam, vbkparam, btcparam);

  ValidationState state;
  EXPECT_TRUE(alttree.btc().bootstrapWithGenesis(state));
  EXPECT_TRUE(alttree.vbk().bootstrapWithGenesis(state));
  EXPECT_TRUE(alttree.bootstrap(state));

  storage.saveBtcTree(alttree.btc());
  storage.saveVbkTree(alttree.vbk());

  BlockTree<BtcBlock, BtcChainParams> reloadedBtcTree{btcparam};
  storage.loadBtcTree(reloadedBtcTree);

  VbkBlockTree reloadedTree{vbkparam, btcparam};
  storage.loadVbkTree(reloadedTree);

  EXPECT_TRUE(reloadedBtcTree == alttree.btc());
}
