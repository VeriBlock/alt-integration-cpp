// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>
#include <veriblock/alt-util.hpp>
#include <veriblock/storage/pop_storage.hpp>

using namespace altintegration;

struct AltTreeRepositoryTest : public ::testing::Test, public PopTestFixture {
  AltTreeRepositoryTest() {}
};

BtcBlock::hash_t lastKnownLocalBtcBlock(const MockMiner& miner) {
  auto tip = miner.btc().getBestChain().tip();
  EXPECT_TRUE(tip);
  return tip->getHash();
}

TEST_F(AltTreeRepositoryTest, Basic) {
  auto* vbkTip = popminer->mineVbkBlocks(1);
  // create endorsement of VBKTIP in BTC_1
  auto btctx = popminer->createBtcTxEndorsingVbkBlock(*vbkTip->header);
  // add BTC tx endorsing VBKTIP into next block
  auto* chainAtip = popminer->mineBtcBlocks(1);

  // create VBK pop tx that has 'block of proof=CHAIN A'
  auto txa = popminer->createVbkPopTxEndorsingVbkBlock(
      *chainAtip->header,
      btctx,
      *vbkTip->header,
      lastKnownLocalBtcBlock(*popminer));
  // erase part of BTC - it will be restored from payloads anyway
  popminer->btc().removeTip(*popminer->btc().getBestChain().tip(), true);

  // mine txA into VBK 2nd block
  vbkTip = popminer->mineVbkBlocks(1);

  PopStorage storage;
  EXPECT_TRUE(popminer->vbk().saveToStorage(storage, state));

  VbkBlockTree reloadedVbkTree{
      vbkparam, btcparam, popminer->vbk().getStoragePayloads()};
  EXPECT_TRUE(reloadedVbkTree.loadFromStorage(storage, state));

  EXPECT_TRUE(reloadedVbkTree.btc() == popminer->btc());
  EXPECT_TRUE(reloadedVbkTree == popminer->vbk());

  popminer->vbk().removeTip(*popminer->vbk().getBestChain().tip(), true);
  EXPECT_FALSE(reloadedVbkTree == popminer->vbk());

  // commands should be properly restored to make it pass
  reloadedVbkTree.removeTip(*reloadedVbkTree.getBestChain().tip(), true);
  EXPECT_TRUE(reloadedVbkTree == popminer->vbk());
  EXPECT_TRUE(reloadedVbkTree.btc() == popminer->btc());
}

TEST_F(AltTreeRepositoryTest, Altchain) {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  // mine 2 blocks
  mineAltBlocks(2, chain);

  AltBlock endorsedBlock = chain[2];

  VbkTx tx = popminer->createVbkTxEndorsingAltBlock(
      generatePublicationData(endorsedBlock));
  AltBlock containingBlock = generateNextBlock(*chain.rbegin());
  chain.push_back(containingBlock);

  PopData altPayloads1 =
      generateAltPayloads({tx}, vbkparam.getGenesisBlock().getHash());

  // mine 1 VBK blocks
  popminer->mineVbkBlocks(1);
  popminer->mineBtcBlocks(1);

  EXPECT_TRUE(alttree.acceptBlock(containingBlock, state));
  EXPECT_TRUE(alttree.addPayloads(containingBlock, altPayloads1, state));
  EXPECT_TRUE(alttree.setState(containingBlock.getHash(), state));
  EXPECT_TRUE(state.IsValid());

  PopStorage storage;
  EXPECT_TRUE(alttree.saveToStorage(storage, state));

  AltTree reloadedAltTree{
      altparam, vbkparam, btcparam, alttree.getStoragePayloads()};
  EXPECT_TRUE(reloadedAltTree.loadFromStorage(storage, state));

  EXPECT_TRUE(reloadedAltTree.vbk().btc() == alttree.vbk().btc());
  EXPECT_TRUE(reloadedAltTree.vbk() == alttree.vbk());
  EXPECT_TRUE(reloadedAltTree == alttree);

  alttree.removeTip(*alttree.getBestChain().tip(), true);
  EXPECT_FALSE(reloadedAltTree == alttree);

  reloadedAltTree.removeTip(*reloadedAltTree.getBestChain().tip(), true);
  EXPECT_TRUE(reloadedAltTree == alttree);
}
