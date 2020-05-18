// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <vector>
#include <ostream>
#include <string>

#include "util/literals.hpp"
#include "veriblock/blob.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/entities/publication_data.hpp"
#include "veriblock/entities/endorsements.hpp"
#include <veriblock/entities/popdata.hpp>
#include <veriblock/entities/payloads.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/blockchain/commands/addblock.hpp>
#include <veriblock/blockchain/commands/addvtb.hpp>
#include <veriblock/blockchain/commands/addendorsement.hpp>
#include <util/alt_chain_params_regtest.hpp>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/rewards/poprewards_bigdecimal.hpp>

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
    NetworkBytePair{false, 0, (uint8_t)TxType::VBK_TX},
    Address::fromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"),
    Coin(1000),
    std::vector<Output>{},
    7,
    publicationData,
    defaultSignature,
    defaultPublicKey};

static const VbkPopTx defaultPoPTx{
    NetworkBytePair{false, 0, (uint8_t)TxType::VBK_POP_TX},
    Address::fromString("VE6MJFzmGdYdrxC8o6UCovVv7BdhdX"),
    {},
    {},
    {},
    {},
    {},
    defaultSignature,
    defaultPublicKey};

TEST(PrettyPrint, PrintEntities) {
  auto blob = Blob<16>("010203");
  EXPECT_EQ(blob.toPrettyString(),
            "Blob<16>(01020300000000000000000000000000)");

  auto btcBlock = BtcBlock{
      536870912u,
      "f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"_unhex,
      "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"_unhex,
      1555501858u,
      436279940u,
      (uint32_t)-1695416274};
  EXPECT_EQ(
      btcBlock.toPrettyString(),
      "BtcBlock{version=536870912, "
      "prev=f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000, "
      "merkleRoot="
      "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e, "
      "timestamp=1555501858, "
      "bits=436279940, nonce=2599551022}");

  auto vbkBlock = VbkBlock{5000,
                           2,
                           "449c60619294546ad825af03"_unhex,
                           "b0935637860679ddd5"_unhex,
                           "5ee4fd21082e18686e"_unhex,
                           "26bbfda7d5e4462ef24ae02d67e47d78"_unhex,
                           1553699059,
                           16842752,
                           1};
  EXPECT_EQ(vbkBlock.toPrettyString(),
            "VbkBlock{height=5000, version=2, "
            "prev=449c60619294546ad825af03, "
            "prevKeystone=b0935637860679ddd5, "
            "secondPrevKeystone=5ee4fd21082e18686e, "
            "merkleRoot="
            "26bbfda7d5e4462ef24ae02d67e47d78, "
            "timestamp=1553699059, "
            "difficulty=16842752, nonce=1}");

  auto atv = ATV{defaultTx, {}, vbkBlock, {}};
  EXPECT_EQ(
      atv.toPrettyString(),
      "ATV{"
      "containingTx="
      "1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22"
      "531c, "
      "containingBlock=a288e74fe0c2e0a6afb0c39808e2aae9a5e19569b1a68624}");

  auto vtb = VTB{defaultPoPTx, {}, vbkBlock, {}};
  EXPECT_EQ(
      vtb.toPrettyString(),
      "VTB{containingTx="
      "6b9d855fdc4c8f0588b960e78f9c2a5650b5597e87e1a4e70b7b9fde5832a9c1, "
      "containingBlock=a288e74fe0c2e0a6afb0c39808e2aae9a5e19569b1a68624}");

  EXPECT_EQ(
      publicationData.toPrettyString(),
      "PublicationData{id=0, header=686561646572206279746573, payoutInfo=7061796f757420696e666f206279746573}");

  auto btcEndorsement = BtcEndorsement::fromContainer(vtb);
  EXPECT_EQ(
      btcEndorsement.toPrettyString(),
      "BtcEndorsement{containing="
      "a288e74fe0c2e0a6afb0c39808e2aae9a5e19569b1a68624, "
      "endorsed=235fce01d9434261188e046ae97aca9edc8530ae042b586c, "
      "endorsedHeight=0, "
      "blockOfProof="
      "14508459b221041eab257d2baaa7459775ba748246c8403609eb708f0e57e74b}");

  auto endorsedBlock = AltBlock{{1, 2, 3}, {4, 5, 6}, 1, 100};
  auto containingBlock = AltBlock{{1, 2, 3}, {4, 5, 6}, 2, 101};
  auto popData = PopData{1, {}, true, atv, {}};
  auto payloads = AltPayloads{endorsedBlock, containingBlock, popData};
  auto vbkEndorsement = VbkEndorsement::fromContainer(payloads);
  EXPECT_EQ(
      vbkEndorsement.toPrettyString(),
      "VbkEndorsement{containing=010203, endorsed=010203, endorsedHeight=100, "
      "blockOfProof=a288e74fe0c2e0a6afb0c39808e2aae9a5e19569b1a68624}");

  auto bigDecimal = PopRewardsBigDecimal(100.0);
  EXPECT_EQ(bigDecimal.toPrettyString(), "BigDecimal{100.0}");
}

TEST(PrettyPrint, PrintCommands) {
  BtcChainParamsRegTest btcparam{};
  VbkChainParamsRegTest vbkparam{};
  AltChainParamsRegTest altparam{};
  BlockTree<BtcBlock, BtcChainParams> btcTree =
      BlockTree<BtcBlock, BtcChainParams>(btcparam);
  BlockTree<VbkBlock, VbkChainParams> vbkTree =
      BlockTree<VbkBlock, VbkChainParams>(vbkparam);
  AltTree alttree = AltTree(altparam, vbkparam, btcparam);

  auto btcBlock = BtcBlock{
      536870912u,
      "f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"_unhex,
      "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"_unhex,
      1555501858u,
      436279940u,
      (uint32_t)-1695416274};
  auto btcBlockPtr = std::make_shared<BtcBlock>(std::move(btcBlock));
  auto addBtcBlock = AddBtcBlock(btcTree, btcBlockPtr);
  EXPECT_EQ(
      addBtcBlock.toPrettyString(),
      "AddBtcBlock{prev="
      "f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000, "
      "block="
      "ebaa22c5ffd827e96c4450ad5dd35dbec2aa45e15cdb5ce9928f543f4cebf10e}");

  auto vbkBlock = VbkBlock{5000,
                           2,
                           "449c60619294546ad825af03"_unhex,
                           "b0935637860679ddd5"_unhex,
                           "5ee4fd21082e18686e"_unhex,
                           "26bbfda7d5e4462ef24ae02d67e47d78"_unhex,
                           1553699059,
                           16842752,
                           1};
  auto vbkBlockPtr = std::make_shared<VbkBlock>(std::move(vbkBlock));
  auto addVbkBlock = AddVbkBlock(vbkTree, vbkBlockPtr);
  EXPECT_EQ(
      addVbkBlock.toPrettyString(),
      "AddVbkBlock{prev=449c60619294546ad825af03, "
      "block=a288e74fe0c2e0a6afb0c39808e2aae9a5e19569b1a68624, height=5000}");

  auto vtb = VTB{defaultPoPTx, {}, vbkBlock, {}};
  auto addVtb = AddVTB(alttree, vtb);
  EXPECT_EQ(addVtb.toPrettyString(), "AddVTB{id=15816616244246527767}");

  auto atv = ATV{defaultTx, {}, vbkBlock, {}};
  auto endorsedBlock = AltBlock{{1, 2, 3}, {4, 5, 6}, 1, 100};
  auto containingBlock = AltBlock{{1, 2, 3}, {4, 5, 6}, 2, 101};
  auto popData = PopData{1, {}, true, atv, {}};
  auto payloads = AltPayloads{endorsedBlock, containingBlock, popData};
  auto vbkEndorsement = VbkEndorsement::fromContainer(payloads);
  auto vbkEndorsementPtr =
      std::make_shared<VbkEndorsement>(std::move(vbkEndorsement));
  auto addEndorsement =
      AddEndorsement<BlockTree<VbkBlock, VbkChainParams>, AltTree>(
          vbkTree, alttree, vbkEndorsementPtr);
  EXPECT_EQ(addEndorsement.toPrettyString(),
            "AddVbkEndorsement{containing=010203, endorsed=010203, "
            "endorsedHeight=100, "
            "blockOfProof=a288e74fe0c2e0a6afb0c39808e2aae9a5e19569b1a68624}");
}

TEST(PrettyPrint, PrintPop) {
  BtcChainParamsRegTest btcparam{};
  VbkChainParamsRegTest vbkparam{};
  AltChainParamsRegTest altparam{};
  auto btcTreePtr =
      std::make_shared<BlockTree<BtcBlock, BtcChainParams>>(btcparam);
  VbkBlockTree vbkTree = VbkBlockTree(vbkparam, btcparam);
  AltTree alttree = AltTree(altparam, vbkparam, btcparam);

  auto comparator =
      PopAwareForkResolutionComparator<VbkBlock,
                                       VbkChainParams,
                                       BlockTree<BtcBlock, BtcChainParams>,
                                       BtcChainParams>(
          btcTreePtr, btcparam, vbkparam);

  EXPECT_EQ(comparator.toPrettyString(),
            "Comparator{\n{tree=\n  BTCBlockTree{blocks=0\n    {tip=<empty>}\n "
            "   {blocks=\n    }\n    {tips=\n    }\n  }}}");

  EXPECT_EQ(
      vbkTree.toPrettyString(),
      "VBKBlockTree{blocks=0\n  {tip=<empty>}\n  {blocks=\n  }\n  {tips=\n  "
      "}\n}\n  Comparator{\n  {tree=\n    BTCBlockTree{blocks=0\n      "
      "{tip=<empty>}\n      {blocks=\n      }\n      {tips=\n      }\n    }}}");
  EXPECT_EQ(btcTreePtr->toPrettyString(),
            "BTCBlockTree{blocks=0\n  {tip=<empty>}\n  {blocks=\n  }\n  "
            "{tips=\n  }\n}");
}

TEST(PrettyPrint, Blockchain) {
  BtcChainParamsRegTest btcparam{};
  VbkChainParamsRegTest vbkparam{};
  AltChainParamsRegTest altparam{};
  auto btcTreePtr =
      std::make_shared<BlockTree<BtcBlock, BtcChainParams>>(btcparam);
  VbkBlockTree vbkTree = VbkBlockTree(vbkparam, btcparam);
  AltTree alttree = AltTree(altparam, vbkparam, btcparam);

  auto btcBlock = BtcBlock{
      536870912u,
      "f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"_unhex,
      "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"_unhex,
      1555501858u,
      436279940u,
      (uint32_t)-1695416274};
  auto btcBlockPtr = std::make_shared<BtcBlock>(std::move(btcBlock));
  BlockIndex<BtcBlock> index{};
  index.height = 100;
  index.header = btcBlockPtr;
  index.pprev = nullptr;

  EXPECT_EQ(
      index.toPrettyString(),
      "BTCBlockIndex{height=100, "
      "hash=ebaa22c5ffd827e96c4450ad5dd35dbec2aa45e15cdb5ce9928f543f4cebf10e, "
      "status=0}");

  EXPECT_EQ(
      alttree.toPrettyString(),
      "AltTree{blocks=0\n  {tip=<empty>}\n  {blocks=\n  }\n  {tips=\n  }\n  "
      "Comparator{\n  {tree=\n    VBKBlockTree{blocks=0\n      {tip=<empty>}\n "
      "     {blocks=\n      }\n      {tips=\n      }\n    }\n      "
      "Comparator{\n      {tree=\n        BTCBlockTree{blocks=0\n          "
      "{tip=<empty>}\n          {blocks=\n          }\n          {tips=\n      "
      "    }\n        }}}}}\n}");
}
