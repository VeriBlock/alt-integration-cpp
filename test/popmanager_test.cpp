#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <random>
#include <veriblock/mock_miner.hpp>
#include <veriblock/popmanager.hpp>
#include <veriblock/state_manager.hpp>
#include <veriblock/storage/endorsement_repository_inmem.hpp>
#include <veriblock/storage/repository_rocks_manager.hpp>

using namespace altintegration;

static const std::string dbName = "db_test";

struct PopManagerTest : public ::testing::Test {
  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;

  std::shared_ptr<EndorsementRepository<BtcEndorsement>> btce;
  std::shared_ptr<EndorsementRepository<VbkEndorsement>> vbke;

  std::shared_ptr<BtcChainParams> btcp =
      std::make_shared<BtcChainParamsRegTest>();
  std::shared_ptr<VbkChainParams> vbkp =
      std::make_shared<VbkChainParamsRegTest>();

  MockMiner apm;
  std::shared_ptr<BtcTree> alt;
  std::shared_ptr<Miner<BtcBlock, BtcChainParams>> alt_miner;

  std::shared_ptr<AltChainParams> altChainParams;

  std::shared_ptr<PopManager> altpop;

  StateManager<RepositoryRocksManager> stateManager;

  PopManagerTest() : stateManager(dbName) {
    altChainParams = std::make_shared<AltChainParams>();

    btce = std::make_shared<EndorsementRepositoryInmem<BtcEndorsement>>();
    vbke = std::make_shared<EndorsementRepositoryInmem<VbkEndorsement>>();

    alt = std::make_shared<BtcTree>(btcp);
    alt_miner = std::make_shared<Miner<BtcBlock, BtcChainParams>>(btcp);
    altpop =
        std::make_shared<PopManager>(btcp, vbkp, btce, vbke, altChainParams);

    // our altchain stores headers of BTC, VBK and ALT blocks
    EXPECT_TRUE(altpop->btc().bootstrapWithGenesis(state));
    EXPECT_TRUE(altpop->vbk().bootstrapWithGenesis(state));
    EXPECT_TRUE(alt->bootstrapWithGenesis(state));

    // also we need to bootstrap APM
    EXPECT_TRUE(apm.btc().bootstrapWithGenesis(state));
    EXPECT_TRUE(apm.vbk().bootstrapWithGenesis(state));
  }

  ValidationState state;
  std::vector<uint8_t> payoutInfo{1, 2, 3, 4, 5};

  template <typename Tree>
  void mineChain(Tree& tree,
                 std::vector<typename Tree::block_t>& blocks,
                 int size) {
    std::generate_n(
        std::back_inserter(blocks), size, [&]() -> typename Tree::block_t {
          auto* tip = tree.getBestChain().tip();
          EXPECT_NE(tip, nullptr);
          auto block = alt_miner->createNextBlock(*tip);
          EXPECT_TRUE(tree.acceptBlock(block, state));
          return block;
        });
  }

  PublicationData endorseBlock(const std::vector<uint8_t>& block) {
    PublicationData pub;
    pub.header = block;
    pub.identifier = 1;
    pub.contextInfo = {};
    pub.payoutInfo = payoutInfo;
    return pub;
  }

  AltBlock makeAltBlock(const BlockIndex<BtcBlock>& block) {
    AltBlock ret;
    ret.timestamp = block.getBlockTime();
    ret.height = block.height;
    ret.hash = block.getHash().asVector();
    return ret;
  }
};

TEST_F(PopManagerTest, Scenario1) {
  std::unique_ptr<StateChange> change = stateManager.newChange();

  // @given: BTC, VBK and ALT chains
  // BTC has genesis + 5 blocks
  EXPECT_TRUE(apm.mineBtcBlocks(5, state));
  EXPECT_EQ(apm.btc().getBestChain().tip()->height, 5);

  // VBK has genesis + 10 blocks
  EXPECT_TRUE(apm.mineVbkBlocks(5, state));
  EXPECT_EQ(apm.vbk().getBestChain().tip()->height, 5);

  // ALT has genesis + 10 blocks
  std::vector<BtcBlock> altfork1{btcp->getGenesisBlock()};
  mineChain(*alt, altfork1, 10);
  ASSERT_EQ(alt->getBestChain().chainHeight(), 10);

  // endorse ALT tip, at height 10
  auto* endorsedAltBlockIndex = alt->getBestChain().tip();
  PublicationData pub = endorseBlock(endorsedAltBlockIndex->header.toRaw());

  // get last known BTC and VBK hashes (current tips)
  auto last_vbk = altpop->vbk().getBestChain().tip()->getHash();
  auto last_btc = altpop->btc().getBestChain().tip()->getHash();

  // mine publications
  auto [atv, vtbs] = apm.mine(pub, last_vbk, last_btc, 1, state);
  ASSERT_TRUE(state.IsValid()) << state.GetRejectReason();

  // mine alt block, which CONTAINS this alt pop tx with endorsement
  mineChain(*alt, altfork1, 1);

  AltProof altProof;
  altProof.atv = atv;
  // endorsed ALT block
  altProof.endorsed = makeAltBlock(*endorsedAltBlockIndex);
  // block that contains current "payloads"
  altProof.containing = makeAltBlock(*alt->getBestChain().tip());

  // apply payloads to our current view
  ASSERT_TRUE(altpop->addPayloads({altProof, vtbs, {}, {}}, *change, state))
      << state.GetRejectReason();
  // these payloads are not committed yet
  ASSERT_TRUE(altpop->hasUncommittedChanges());

  // now, our local view on btc/vbk chains is correct
  ASSERT_EQ(*altpop->btc().getBestChain().tip(),
            *apm.btc().getBestChain().tip()->pprev);
  ASSERT_EQ(*altpop->vbk().getBestChain().tip(),
            *apm.vbk().getBestChain().tip()->pprev);

  // rollback last addPayloads
  altpop->rollback(*change);

  // our local view changed to previous state
  ASSERT_EQ(altpop->btc().getBestChain().tip()->getHash(), last_btc);
  ASSERT_EQ(altpop->vbk().getBestChain().tip()->getHash(), last_vbk);

  // add same payloads again
  ASSERT_TRUE(altpop->addPayloads({altProof, vtbs, {}, {}}, *change, state))
      << state.GetRejectReason();
  // these payloads are not committed yet
  ASSERT_TRUE(altpop->hasUncommittedChanges());
  // commit current payloads
  altpop->commit();
  // now we don't have uncommitted changes
  ASSERT_FALSE(altpop->hasUncommittedChanges());

  // now, our local view on btc/vbk chains is correct
  ASSERT_EQ(*altpop->btc().getBestChain().tip(),
            *apm.btc().getBestChain().tip()->pprev);
  ASSERT_EQ(*altpop->vbk().getBestChain().tip(),
            *apm.vbk().getBestChain().tip()->pprev);

  // finally, do rollback again. this should be NOOP
  altpop->rollback(*change);

  // our local view on btc/vbk chains is still correct
  ASSERT_EQ(*altpop->btc().getBestChain().tip(),
            *apm.btc().getBestChain().tip()->pprev);
  ASSERT_EQ(*altpop->vbk().getBestChain().tip(),
            *apm.vbk().getBestChain().tip()->pprev);
}

TEST_F(PopManagerTest, compareTwoBranches_test) {
  std::unique_ptr<StateChange> change = stateManager.newChange();
  // ALT has genesis + 102 blocks
  std::vector<BtcBlock> altfork1{btcp->getGenesisBlock()};
  mineChain(*alt, altfork1, 102);
  ASSERT_EQ(alt->getBestChain().chainHeight(), 102);

  std::vector<BtcBlock> altfork2 = altfork1;
  altfork2.resize(100);

  EXPECT_EQ(altfork1.size(), 103);
  EXPECT_EQ(altfork2.size(), 100);

  // endorse ALT tip, at height 102
  auto* endorsedAltBlockIndex = alt->getBestChain().tip();
  PublicationData pub = endorseBlock(endorsedAltBlockIndex->header.toRaw());

  // get last known BTC and VBK hashes (current tips)
  auto last_vbk = altpop->vbk().getBestChain().tip()->getHash();
  auto last_btc = altpop->btc().getBestChain().tip()->getHash();

  // mine publications
  auto [atv, vtbs] = apm.mine(pub, last_vbk, last_btc, 1, state);
  ASSERT_TRUE(state.IsValid()) << state.GetRejectReason();

  // mine alt block, which CONTAINS this alt pop tx with endorsement
  mineChain(*alt, altfork1, 1);

  AltProof altProof;
  altProof.atv = atv;
  // endorsed ALT block
  altProof.endorsed = makeAltBlock(*endorsedAltBlockIndex);
  // block that contains current "payloads"
  altProof.containing = makeAltBlock(*alt->getBestChain().tip());

  // apply payloads to our current view
  ASSERT_TRUE(altpop->addPayloads({altProof, vtbs, {}, {}}, *change, state))
      << state.GetRejectReason();

  altpop->commit();

  // mine
  alt->invalidateBlockByHash(altfork1[100].getHash());
  EXPECT_EQ(alt->getBestChain().blocksCount(), 100);
  EXPECT_EQ(alt->getBestChain().tip()->getHash(),
            altfork2[altfork2.size() - 1].getHash());

  mineChain(*alt, altfork2, 8);

  EXPECT_EQ(altfork2[99].getHash().asVector(),
            altfork1[99].getHash().asVector());

  BlockIndex<AltBlock> index_prev;
  index_prev.header = {
      altfork1[99].getHash().asVector(), {}, altfork1[99].getBlockTime(), 99};
  index_prev.height = 99;
  index_prev.pprev = nullptr;

  Chain<AltBlock> chain1(index_prev.height, &index_prev);
  Chain<AltBlock> chain2(index_prev.height, &index_prev);

  std::vector<std::unique_ptr<BlockIndex<AltBlock>>> alt1;
  for (size_t i = 100; i < altfork1.size(); i++) {
    AltBlock block{altfork1[i].getHash().asVector(),
                   {},
                   altfork1[i].getBlockTime(),
                   (int32_t)i};

    auto* index = new BlockIndex<AltBlock>();
    index->header = block;
    index->pprev = chain1.tip();
    index->height = (int32_t)i;
    chain1.setTip(index);
    alt1.emplace_back(index);
  }

  std::vector<std::unique_ptr<BlockIndex<AltBlock>>> alt2;
  for (size_t i = 100; i < altfork2.size(); i++) {
    AltBlock block{altfork2[i].getHash().asVector(),
                   {},
                   altfork2[i].getBlockTime(),
                   (int32_t)i};

    auto* index = new BlockIndex<AltBlock>();
    index->header = block;
    index->pprev = chain2.tip();
    index->height = (int32_t)i;
    chain2.setTip(index);
    alt2.emplace_back(index);
  }

  EXPECT_GT(altpop->compareTwoBranches(chain1, chain2), 0);
  EXPECT_LT(altpop->compareTwoBranches(chain2, chain1), 0);
}
