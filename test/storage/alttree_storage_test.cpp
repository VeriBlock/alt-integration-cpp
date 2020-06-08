// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/storage/pop_storage.hpp>
#include <util/pop_test_fixture.hpp>
#include <veriblock/alt-util.hpp>

using namespace altintegration;

struct AltTreeRepositoryTest : public ::testing::Test, public PopTestFixture {
  AltTreeRepositoryTest() {
  }
};

BtcBlock::hash_t lastKnownLocalBtcBlock(const MockMiner& miner) {
  auto tip = miner.btc().getBestChain().tip();
  EXPECT_TRUE(tip);
  return tip->getHash();
}

TEST_F(AltTreeRepositoryTest, Basic) {
  PopStorage storage{};

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

  storage.saveBtcTree(popminer->btc());
  storage.saveVbkTree(popminer->vbk());

  BlockTree<BtcBlock, BtcChainParams> reloadedBtcTree{btcparam};
  storage.loadBtcTree(reloadedBtcTree);

  VbkBlockTree reloadedVbkTree{vbkparam, btcparam, storage.vbkEndorsements()};
  storage.loadVbkTree(reloadedVbkTree);

  EXPECT_TRUE(reloadedBtcTree == popminer->btc());
  EXPECT_TRUE(reloadedVbkTree == popminer->vbk());

  popminer->vbk().removeTip(*popminer->vbk().getBestChain().tip(), true);
  EXPECT_FALSE(reloadedVbkTree == popminer->vbk());

  // commands should be properly restored to make it pass
  reloadedVbkTree.removeTip(*reloadedVbkTree.getBestChain().tip(), true);
  EXPECT_TRUE(reloadedVbkTree == popminer->vbk());
  EXPECT_TRUE(reloadedVbkTree.btc() == popminer->btc());
}
