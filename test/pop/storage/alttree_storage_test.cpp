// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <pop/util/pop_test_fixture.hpp>
#include <veriblock/pop/blockchain/block_index.hpp>
#include <veriblock/pop/storage/util.hpp>

using namespace altintegration;

struct AltTreeRepositoryTest : public ::testing::Test, public PopTestFixture {
  PayloadsIndex payloadsIndex;
};

BtcBlock::hash_t lastKnownLocalBtcBlock(const MockMiner& miner) {
  auto tip = miner.btc().getBestChain().tip();
  EXPECT_TRUE(tip);
  return tip->getHash();
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

  save(alttree);

  AltBlockTree reloadedAltTree{this->altparam,
                               this->vbkparam,
                               this->btcparam,
                               this->payloadsProvider,
                               this->blockProvider};

  reloadedAltTree.btc().bootstrapWithGenesis(GetRegTestBtcBlock());
  reloadedAltTree.vbk().bootstrapWithGenesis(GetRegTestVbkBlock());
  reloadedAltTree.bootstrap();

  ASSERT_TRUE(loadTrees(reloadedAltTree, state));
  ASSERT_TRUE(this->cmp(reloadedAltTree.vbk().btc(), this->alttree.btc()))
      << "initial : \n"
      << alttree.toPrettyString() << "\n\n"
      << "reloaded: \n"
      << reloadedAltTree.toPrettyString();

  ASSERT_TRUE(this->cmp(reloadedAltTree.vbk(), this->alttree.vbk()));
  ASSERT_TRUE(this->cmp(reloadedAltTree, this->alttree));

  ASSERT_TRUE(this->cmp(reloadedAltTree.btc(), this->alttree.btc()))
      << "initial : \n"
      << alttree.toPrettyString() << "\n\n"
      << "reloaded: \n"
      << reloadedAltTree.toPrettyString();
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

  save(alttree);

  AltBlockTree reloadedAltTree{this->altparam,
                               this->vbkparam,
                               this->btcparam,
                               this->payloadsProvider,
                               this->blockProvider};

  reloadedAltTree.btc().bootstrapWithGenesis(GetRegTestBtcBlock());
  reloadedAltTree.vbk().bootstrapWithGenesis(GetRegTestVbkBlock());
  reloadedAltTree.bootstrap();

  ASSERT_TRUE(loadTrees(reloadedAltTree, state));

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

  auto vtb = this->popminer->createVTB(vbkTip->getHeader(), vbkpoptx);

  PopData popData;
  popData.vtbs = {vtb};
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

  save(alttree);

  AltBlockTree reloadedAltTree{this->altparam,
                               this->vbkparam,
                               this->btcparam,
                               this->payloadsProvider,
                               this->blockProvider};

  reloadedAltTree.btc().bootstrapWithGenesis(GetRegTestBtcBlock());
  reloadedAltTree.vbk().bootstrapWithGenesis(GetRegTestVbkBlock());
  reloadedAltTree.bootstrap();

  ASSERT_TRUE(loadTrees(reloadedAltTree, state));

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

TEST_F(AltTreeRepositoryTest, SaveAfterSave) {
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
  save(alttree);

  // rollback 1 block and save again
  EXPECT_TRUE(
      this->alttree.setState(containingBlock.getPreviousBlock(), this->state));
  EXPECT_TRUE(this->state.IsValid());
  save(alttree);

  AltBlockTree reloadedAltTree{this->altparam,
                               this->vbkparam,
                               this->btcparam,
                               this->payloadsProvider,
                               this->blockProvider};

  reloadedAltTree.btc().bootstrapWithGenesis(GetRegTestBtcBlock());
  reloadedAltTree.vbk().bootstrapWithGenesis(GetRegTestVbkBlock());
  reloadedAltTree.bootstrap();

  ASSERT_TRUE(loadTrees(reloadedAltTree, state));
}
