// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/storage/util.hpp>

using namespace altintegration;

struct AltTreeRepositoryTest : public ::testing::Test, public PopTestFixture {
  PayloadsIndex payloadsIndex;
};

BtcBlock::hash_t lastKnownLocalBtcBlock(const MockMiner2& miner) {
  auto tip = miner.btc().getBestChain().tip();
  EXPECT_TRUE(tip);
  return tip->getHash();
}

TEST_F(AltTreeRepositoryTest, ValidBlocks) {
  auto* vbkTip = this->popminer->mineVbkBlocks(1);
  // create endorsement of VBKTIP in BTC_1
  auto btctx =
      this->popminer->createBtcTxEndorsingVbkBlock(vbkTip->getHeader());
  // add BTC tx endorsing VBKTIP into next block
  auto* chainAtip = this->popminer->mineBtcBlocks(1, {btctx});

  // create VBK pop tx that has 'block of proof=CHAIN A'
  auto vbkpoptx = this->popminer->createVbkPopTxEndorsingVbkBlock(
      chainAtip->getHeader(),
      btctx,
      vbkTip->getHeader(),
      lastKnownLocalBtcBlock(*this->popminer));
  // erase part of BTC - it will be restored from payloads anyway
  this->popminer->btc().removeLeaf(*this->popminer->btc().getBestChain().tip());

  // mine txA into VBK 2nd block
  this->popminer->mineVbkBlocks(1, {vbkpoptx});

  auto writer = InmemBlockBatch(blockStorage);
  SaveTree(this->popminer->btc(), writer);
  SaveTree(this->popminer->vbk(), writer);

  VbkBlockTree newvbk{this->vbkparam,
                      this->btcparam,
                      popminer->getPayloadsProvider(),
                      payloadsIndex};
  newvbk.btc().bootstrapWithGenesis(GetRegTestBtcBlock(), this->state);
  newvbk.bootstrapWithGenesis(GetRegTestVbkBlock(), this->state);

  ASSERT_TRUE(LoadTreeWrapper(newvbk.btc())) << this->state.toString();
  ASSERT_TRUE(LoadTreeWrapper(newvbk)) << this->state.toString();

  ASSERT_TRUE(this->cmp(newvbk.btc(), this->popminer->btc()));
  ASSERT_TRUE(this->cmp(newvbk, this->popminer->vbk()));
  this->popminer->vbk().removeLeaf(*this->popminer->vbk().getBestChain().tip());
  ASSERT_FALSE(this->cmp(newvbk, this->popminer->vbk(), true));

  // commands should be properly restored to make it pass
  newvbk.removeLeaf(*newvbk.getBestChain().tip());
  ASSERT_TRUE(this->cmp(newvbk, this->popminer->vbk()));
  ASSERT_TRUE(this->cmp(newvbk.btc(), this->popminer->btc()));
}

TEST_F(AltTreeRepositoryTest, Altchain) {
  std::vector<AltBlock> chain = {this->altparam.getBootstrapBlock()};

  // mine 2 blocks
  this->mineAltBlocks(2, chain);

  AltBlock endorsedBlock = chain[2];

  VbkTx tx = this->popminer->createVbkTxEndorsingAltBlock(
      this->generatePublicationData(endorsedBlock));
  AltBlock containingBlock = this->generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  PopData altPayloads1 =
      this->generateAltPayloads({tx}, GetRegTestVbkBlock().getHash());

  // mine 1 VBK blocks
  this->popminer->mineVbkBlocks(1);
  this->popminer->mineBtcBlocks(1);

  EXPECT_TRUE(this->alttree.acceptBlockHeader(containingBlock, this->state));
  EXPECT_TRUE(this->AddPayloads(containingBlock.getHash(), altPayloads1));
  EXPECT_TRUE(this->alttree.setState(containingBlock.getHash(), this->state));
  EXPECT_TRUE(this->state.IsValid());

  auto writer = InmemBlockBatch(blockStorage);
  SaveAllTrees(this->alttree, writer);

  AltBlockTree reloadedAltTree{
      this->altparam, this->vbkparam, this->btcparam, payloadsProvider};

  reloadedAltTree.btc().bootstrapWithGenesis(GetRegTestBtcBlock(), this->state);
  reloadedAltTree.vbk().bootstrapWithGenesis(GetRegTestVbkBlock(), this->state);
  bool bootstrapped = reloadedAltTree.bootstrap(this->state);
  ASSERT_TRUE(bootstrapped);

  ASSERT_TRUE(LoadTreeWrapper(reloadedAltTree.btc()));
  ASSERT_TRUE(LoadTreeWrapper(reloadedAltTree.vbk()));
  ASSERT_TRUE(LoadTreeWrapper(reloadedAltTree));

  ASSERT_TRUE(
      this->cmp(reloadedAltTree.vbk().btc(), this->alttree.vbk().btc()));
  ASSERT_TRUE(this->cmp(reloadedAltTree.vbk(), this->alttree.vbk()));
  ASSERT_TRUE(this->cmp(reloadedAltTree, this->alttree));

  this->alttree.removeLeaf(*this->alttree.getBestChain().tip());
  EXPECT_FALSE(this->cmp(reloadedAltTree, this->alttree, true));

  reloadedAltTree.removeLeaf(*reloadedAltTree.getBestChain().tip());
  EXPECT_TRUE(this->cmp(reloadedAltTree, this->alttree));
}

TEST_F(AltTreeRepositoryTest, ManyEndorsements) {
  std::vector<AltBlock> chain = {this->altparam.getBootstrapBlock()};

  // mine 2 blocks
  this->mineAltBlocks(2, chain);

  AltBlock endorsedBlock1 = chain[1];
  AltBlock endorsedBlock2 = chain[2];

  VbkTx tx1 = this->popminer->createVbkTxEndorsingAltBlock(
      this->generatePublicationData(endorsedBlock1));
  VbkTx tx2 = this->popminer->createVbkTxEndorsingAltBlock(
      this->generatePublicationData(endorsedBlock2));
  AltBlock containingBlock = this->generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  PopData altPayloads1 =
      this->generateAltPayloads({tx1, tx2}, GetRegTestVbkBlock().getHash());

  // mine 1 VBK blocks
  this->popminer->mineVbkBlocks(1);
  this->popminer->mineBtcBlocks(1);

  EXPECT_TRUE(this->alttree.acceptBlockHeader(containingBlock, this->state));
  EXPECT_TRUE(this->AddPayloads(containingBlock.getHash(), altPayloads1));
  EXPECT_TRUE(this->alttree.setState(containingBlock.getHash(), this->state));
  EXPECT_TRUE(this->state.IsValid());

  auto writer = InmemBlockBatch(blockStorage);
  SaveAllTrees(this->alttree, writer);

  AltBlockTree reloadedAltTree{
      this->altparam, this->vbkparam, this->btcparam, payloadsProvider};

  reloadedAltTree.btc().bootstrapWithGenesis(GetRegTestBtcBlock(), this->state);
  reloadedAltTree.vbk().bootstrapWithGenesis(GetRegTestVbkBlock(), this->state);
  ASSERT_TRUE(reloadedAltTree.bootstrap(this->state));

  ASSERT_TRUE(LoadTreeWrapper(reloadedAltTree.btc()));
  ASSERT_TRUE(LoadTreeWrapper(reloadedAltTree.vbk()));
  ASSERT_TRUE(LoadTreeWrapper(reloadedAltTree));

  ASSERT_TRUE(
      this->cmp(reloadedAltTree.vbk().btc(), this->alttree.vbk().btc()));
  ASSERT_TRUE(this->cmp(reloadedAltTree.vbk(), this->alttree.vbk()));
  ASSERT_TRUE(this->cmp(reloadedAltTree, this->alttree));
}

TEST_F(AltTreeRepositoryTest, InvalidBlocks) {
  std::vector<AltBlock> chain = {this->altparam.getBootstrapBlock()};

  VBK_LOG_DEBUG("mine 20 alt blocks");
  this->mineAltBlocks(20, chain);

  auto* vbkTip = this->popminer->mineVbkBlocks(1);
  VBK_LOG_DEBUG("create an endorsement of VBKTIP in BTC_1");
  auto btctx =
      this->popminer->createBtcTxEndorsingVbkBlock(vbkTip->getHeader());
  VBK_LOG_DEBUG("add a BTC tx endorsing VBKTIP to the next block");
  auto* chainAtip = this->popminer->mineBtcBlocks(1, {btctx});

  VBK_LOG_DEBUG("create a VBK PoP tx that has 'block of proof=CHAIN A'");
  auto vbkpoptx = this->popminer->createVbkPopTxEndorsingVbkBlock(
      chainAtip->getHeader(),
      btctx,
      vbkTip->getHeader(),
      lastKnownLocalBtcBlock(*this->popminer));

  // mine txA into VBK 2nd block
  vbkTip = this->popminer->mineVbkBlocks(1, {vbkpoptx});

  auto vtbs = this->popminer->vbkPayloads[vbkTip->getHash()];

  ASSERT_EQ(vtbs.size(), 1);

  PopData popData;
  popData.vtbs = vtbs;
  this->fillVbkContext(
      popData.context, GetRegTestVbkBlock().getHash(), this->popminer->vbk());
  auto containingBlock = this->generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  VBK_LOG_DEBUG("add alt payloads");
  EXPECT_TRUE(this->alttree.acceptBlockHeader(containingBlock, this->state));
  EXPECT_TRUE(this->AddPayloads(containingBlock.getHash(), popData));
  EXPECT_TRUE(this->alttree.setState(containingBlock.getHash(), this->state));
  EXPECT_TRUE(this->state.IsValid());
  validateAlttreeIndexState(this->alttree, containingBlock, popData);

  popData.context.clear();
  // corrupt vtb
  popData.vtbs[0].containingBlock.setMerkleRoot(uint128());

  containingBlock = this->generateNextBlock(chain.back());
  chain.push_back(containingBlock);

  VBK_LOG_DEBUG("add alt payloads #2");
  EXPECT_TRUE(this->alttree.acceptBlockHeader(containingBlock, this->state));
  EXPECT_TRUE(this->AddPayloads(containingBlock.getHash(), popData));
  EXPECT_FALSE(this->alttree.setState(containingBlock.getHash(), this->state));
  EXPECT_FALSE(this->state.IsValid());

  // all payloads are marked valid as there's no correctly implemented
  // invalidation
  validateAlttreeIndexState(
      this->alttree, containingBlock, popData, /*payloads_validation =*/true);

  auto writer = InmemBlockBatch(blockStorage);
  SaveAllTrees(this->alttree, writer);

  AltBlockTree reloadedAltTree{
      this->altparam, this->vbkparam, this->btcparam, payloadsProvider};

  reloadedAltTree.btc().bootstrapWithGenesis(GetRegTestBtcBlock(), this->state);
  reloadedAltTree.vbk().bootstrapWithGenesis(GetRegTestVbkBlock(), this->state);
  ASSERT_TRUE(reloadedAltTree.bootstrap(this->state));

  ASSERT_TRUE(LoadTreeWrapper(reloadedAltTree.btc()));
  ASSERT_TRUE(LoadTreeWrapper(reloadedAltTree.vbk()));
  ASSERT_TRUE(LoadTreeWrapper(reloadedAltTree));

  ASSERT_TRUE(
      this->cmp(reloadedAltTree.vbk().btc(), this->alttree.vbk().btc()));
  ASSERT_TRUE(this->cmp(reloadedAltTree.vbk(), this->alttree.vbk()));
  // all payloads are marked valid as there's no correctly implemented
  // invalidation thus the reloaded tree has to have identical contents
  EXPECT_TRUE(this->cmp(reloadedAltTree, this->alttree));

  VBK_LOG_DEBUG("set state so that validity flags end up to be the same");
  EXPECT_FALSE(
      reloadedAltTree.setState(containingBlock.getHash(), this->state));
  EXPECT_FALSE(this->alttree.setState(containingBlock.getHash(), this->state));

  // FIXME: we no longer attempt to apply known invalid blocks, and since
  // payload validity is not persisted, this comparison fails
  // ASSERT_TRUE(this->cmp(reloadedAltTree, this->alttree));
}
