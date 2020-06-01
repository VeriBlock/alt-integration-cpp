// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/blockchain/commands/commands.hpp>

#include "util/pop_test_fixture.hpp"

using namespace altintegration;

struct AtomicityTestFixture : public ::testing::Test, public PopTestFixture {};

TEST_F(AtomicityTestFixture, AddVbkEndorsement) {
  popminer->mineBtcBlocks(10);
  auto vbktip = popminer->mineVbkBlocks(10);
  // endorsed
  auto vbk5 = vbktip->getAncestor(5);
  ASSERT_TRUE(vbk5);
  // containing
  auto vbk10 = vbktip->getAncestor(10);
  ASSERT_TRUE(vbk10);

  auto e = std::make_shared<VbkEndorsement>();
  e->id = uint256::fromHex("1");
  e->blockOfProof = popminer->btc().getBestChain().tip()->getHash();
  e->endorsedHeight = vbk5->height;
  e->endorsedHash = vbk5->getHash();
  e->containingHash = vbk10->getHash();

  auto cmd =
      std::make_shared<AddVbkEndorsement>(popminer->btc(), popminer->vbk(), e);

  // before cmd execution we have 0 endorsements
  ASSERT_EQ(vbk5->containingEndorsements.size(), 0);
  ASSERT_EQ(vbk5->endorsedBy.size(), 0);
  ASSERT_EQ(vbk10->containingEndorsements.size(), 0);
  ASSERT_EQ(vbk10->endorsedBy.size(), 0);

  // execute command
  ASSERT_TRUE(cmd->Execute(state)) << state.toString();

  // verify that state has been changed
  ASSERT_EQ(vbk5->containingEndorsements.size(), 0);
  ASSERT_EQ(vbk5->endorsedBy.size(), 1);
  ASSERT_EQ(vbk10->containingEndorsements.size(), 1);
  ASSERT_EQ(vbk10->endorsedBy.size(), 0);
  ASSERT_EQ(e->refs, 0);

  {
    // TODO: remove second cmd Exec/ after VTB duplication is disabled

    // execute command second time
    ASSERT_TRUE(cmd->Execute(state));

    // verify that state has been changed
    ASSERT_EQ(vbk5->containingEndorsements.size(), 0);
    ASSERT_EQ(vbk5->endorsedBy.size(), 1);
    ASSERT_EQ(vbk10->containingEndorsements.size(), 1);
    ASSERT_EQ(vbk10->endorsedBy.size(), 0);
    ASSERT_EQ(e->refs, 1);

    // unexecute command first time (removes duplicate)
    ASSERT_NO_FATAL_FAILURE(cmd->UnExecute());

    ASSERT_EQ(vbk5->containingEndorsements.size(), 0);
    ASSERT_EQ(vbk5->endorsedBy.size(), 1);
    ASSERT_EQ(vbk10->containingEndorsements.size(), 1);
    ASSERT_EQ(vbk10->endorsedBy.size(), 0);
    ASSERT_EQ(e->refs, 0);  // now refs == 0
  }

  // unexecute command
  ASSERT_NO_FATAL_FAILURE(cmd->UnExecute());

  // endorsement is removed
  ASSERT_EQ(vbk5->containingEndorsements.size(), 0);
  ASSERT_EQ(vbk5->endorsedBy.size(), 0);
  ASSERT_EQ(vbk10->containingEndorsements.size(), 0);
  ASSERT_EQ(vbk10->endorsedBy.size(), 0);

  ASSERT_DEATH(cmd->UnExecute(), "");
}

TEST_F(AtomicityTestFixture, AddAltEndorsement) {
  popminer->mineBtcBlocks(10);
  popminer->mineVbkBlocks(10);
  std::vector<AltBlock> chain{altparam.getBootstrapBlock()};
  mineAltBlocks(10, chain);

  // alt tip = last alt block in "chain"
  ASSERT_TRUE(alttree.setState(chain.rbegin()->getHash(), state));

  // endorsed
  auto alt5 = alttree.getBestChain().tip()->getAncestor(5);
  ASSERT_TRUE(alt5);
  // containing
  auto alt10 = alttree.getBestChain().tip()->getAncestor(10);
  ASSERT_TRUE(alt10);

  auto e = std::make_shared<AltEndorsement>();
  e->id = uint256::fromHex("1");
  e->blockOfProof = popminer->vbk().getBestChain().tip()->getHash();
  e->endorsedHeight = alt5->height;
  e->endorsedHash = alt5->getHash();
  e->containingHash = alt10->getHash();

  auto cmd = std::make_shared<AddAltEndorsement>(popminer->vbk(), alttree, e);

  // before cmd execution we have 0 endorsements
  ASSERT_EQ(alt5->containingEndorsements.size(), 0);
  ASSERT_EQ(alt5->endorsedBy.size(), 0);
  ASSERT_EQ(alt10->containingEndorsements.size(), 0);
  ASSERT_EQ(alt10->endorsedBy.size(), 0);

  // execute command
  ASSERT_TRUE(cmd->Execute(state)) << state.toString();

  // verify that state has been changed
  ASSERT_EQ(alt5->containingEndorsements.size(), 0);
  ASSERT_EQ(alt5->endorsedBy.size(), 1);
  ASSERT_EQ(alt10->containingEndorsements.size(), 1);
  ASSERT_EQ(alt10->endorsedBy.size(), 0);
  ASSERT_EQ(e->refs, 0);

  // execute command second time
  ASSERT_FALSE(cmd->Execute(state));

  // verify that state has NOT been changed
  ASSERT_EQ(alt5->containingEndorsements.size(), 0);
  ASSERT_EQ(alt5->endorsedBy.size(), 1);
  ASSERT_EQ(alt10->containingEndorsements.size(), 1);
  ASSERT_EQ(alt10->endorsedBy.size(), 0);
  ASSERT_EQ(e->refs, 0);

  // unexecute command
  ASSERT_NO_FATAL_FAILURE(cmd->UnExecute());

  // endorsement is removed
  ASSERT_EQ(alt5->containingEndorsements.size(), 0);
  ASSERT_EQ(alt5->endorsedBy.size(), 0);
  ASSERT_EQ(alt10->containingEndorsements.size(), 0);
  ASSERT_EQ(alt10->endorsedBy.size(), 0);

  ASSERT_DEATH(cmd->UnExecute(), "");
}

TEST_F(AtomicityTestFixture, AddVTB) {
  popminer->mineBtcBlocks(10);
  popminer->mineVbkBlocks(10);

  // create a single VTB which has endorsed VBK5, containing - VBK11
  // endorsed
  auto vbk5 = popminer->vbk().getBestChain().tip()->getAncestor(5);
  ASSERT_TRUE(vbk5);
  auto vbkpoptx1 =
      popminer->endorseVbkBlock(*vbk5->header, getLastKnownBtcBlock(), state);
  auto vbkpoptx2 =
      popminer->endorseVbkBlock(*vbk5->header, getLastKnownBtcBlock(), state);
  popminer->vbkmempool.push_back(vbkpoptx1);
  popminer->vbkmempool.push_back(vbkpoptx2);
  auto vbkcontaining = popminer->mineVbkBlocks(1);
  ASSERT_TRUE(popminer->vbkmempool.empty());

  // now we have 2 valid VTBs endorsing VBK5
  VTB& vtb1 = popminer->vbkPayloads.at(vbkcontaining->getHash()).at(0);
  VTB& vtb2 = popminer->vbkPayloads.at(vbkcontaining->getHash()).at(1);
  (void) vtb2;

  auto cmd1 = std::make_shared<AddVTB>(alttree, vtb1);

  // before execution, we have to supply VBK blocks context to ALT tree...
  // bypass all AltPayloads, and add them directly
  for (auto* block : popminer->vbk().getBestChain()) {
    if (block->pprev == nullptr) {
      // skip genesis block
      continue;
    }

    ASSERT_TRUE(alttree.vbk().acceptBlock(block->header, state));
  }

  // verify VBK tree does not know about this VTB
  auto altvbkcontaining = alttree.vbk().getBlockIndex(vtb1.containingBlock.getHash());
  ASSERT_TRUE(altvbkcontaining);

  ASSERT_EQ(altvbkcontaining->commands.size(), 0);

  // execute that AddVTB1
  ASSERT_TRUE(cmd1->Execute(state)) << state.toString();

  ASSERT_EQ(altvbkcontaining->commands.size(), 1);
  ASSERT_EQ(altvbkcontaining->commands.at(0).id, vtb1.getId());

  {
    // TODO: remove this when VTB duplication check is added
    // run execute second time on same VTB
    ASSERT_TRUE(cmd1->Execute(state));

    ASSERT_EQ(altvbkcontaining->commands.size(), 2);
    ASSERT_EQ(altvbkcontaining->commands.at(0).id, vtb1.getId());
    ASSERT_EQ(altvbkcontaining->commands.at(1).id, vtb1.getId());
  }

  // add vtb2
  auto cmd2 = std::make_shared<AddVTB>(alttree, vtb2);
  ASSERT_TRUE(cmd2->Execute(state));

  ASSERT_EQ(altvbkcontaining->commands.size(), 3);
  ASSERT_EQ(altvbkcontaining->commands.at(0).id, vtb1.getId());
  ASSERT_EQ(altvbkcontaining->commands.at(1).id, vtb1.getId());
  ASSERT_EQ(altvbkcontaining->commands.at(2).id, vtb2.getId());

  // unexecute VTB2
  ASSERT_NO_FATAL_FAILURE(cmd2->UnExecute());

  ASSERT_EQ(altvbkcontaining->commands.size(), 2);
  ASSERT_EQ(altvbkcontaining->commands.at(0).id, vtb1.getId());
  ASSERT_EQ(altvbkcontaining->commands.at(1).id, vtb1.getId());

  // unexecute first VTB1
  ASSERT_NO_FATAL_FAILURE(cmd1->UnExecute());
  ASSERT_EQ(altvbkcontaining->commands.size(), 1);
  ASSERT_EQ(altvbkcontaining->commands.at(0).id, vtb1.getId());

  // unexecute second VTB1
  ASSERT_NO_FATAL_FAILURE(cmd1->UnExecute());
  ASSERT_EQ(altvbkcontaining->commands.size(), 0);
}