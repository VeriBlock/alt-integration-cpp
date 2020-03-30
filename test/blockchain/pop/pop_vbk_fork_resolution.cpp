#include "util/pop_test_fixture.hpp"
#include "util/visualize.hpp"

using namespace altintegration;

struct PopVbkForkResolution : public ::testing::Test {
  MockMiner popminer;
};

TEST_F(PopVbkForkResolution, A_1_endorsement_B_longer) {
  // start with 10 BTC blocks
  popminer.mineBtcBlocks(10);
  // start with 65 VBK blocks
  auto* chainBtip = popminer.mineVbkBlocks(65);

  // current best chain is at block 65, chain B
  ASSERT_EQ(popminer.vbk().getBestChain().tip(), chainBtip);

  auto* forkPoint = popminer.vbk().getBestChain().tip()->getAncestor(50);
  auto* chainAtip = popminer.mineVbkBlocks(*forkPoint, 10);

  // best chain is still B
  ASSERT_EQ(popminer.vbk().getBestChain().tip(), chainBtip);

  // create 1 endorsement and put it into
  auto Atx1 = popminer.createBtcTxEndorsingVbkBlock(chainAtip->header);
  auto Abtccontaining1 = popminer.mineBtcBlocks(1);
  ASSERT_TRUE(popminer.btc().getBestChain().contains(Abtccontaining1));

  auto Apoptx1 = popminer.createVbkPopTxEndorsingVbkBlock(
      Abtccontaining1->header,
      Atx1,
      chainAtip->header,
      popminer.btc().getBestChain().tip()->getHash());

  // state is still at chain B
  ASSERT_EQ(popminer.vbk().getBestChain().tip(), chainBtip);

  // mine one block on top of smaller chain A.
  // this block will contain endorsement of chain A
  auto Avbkcontaining1 = popminer.mineVbkBlocks(*chainAtip, 1);

  // chain changed to chain A, because its POP score is higher
  ASSERT_EQ(popminer.vbk().getBestChain().tip(), Avbkcontaining1);

  // TODO: fix

  // and now endorse block 60 of chain B
  auto* B60 = chainBtip->getAncestor(60);
  auto Btx1 = popminer.createBtcTxEndorsingVbkBlock(B60->header);
  auto Bbtccontaining1 = popminer.mineBtcBlocks(1);
  ASSERT_TRUE(popminer.btc().getBestChain().contains(Bbtccontaining1));

  auto Bpoptx1 = popminer.createVbkPopTxEndorsingVbkBlock(
      Bbtccontaining1->header,
      Btx1,
      B60->header,
      popminer.btc().getBestChain().tip()->getHash());

  popminer.mineVbkBlocks(*B60, 1);

  // chain is still at chain A, because endorsement was erlier
  ASSERT_EQ(popminer.vbk().getBestChain().tip(), Avbkcontaining1);

  std::ofstream f("file.txt");
  WriteBlockTree(f, popminer.vbk(), "clusterVBK");
  WriteBlockTree(f, popminer.btc(), "clusterBTC");
  f.close();
}
