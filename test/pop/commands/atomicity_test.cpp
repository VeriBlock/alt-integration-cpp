// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/blockchain/commands/commands.hpp>

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
  e->endorsedHash = vbk5->getHash();
  e->containingHash = vbk10->getHash();

  auto cmd =
      std::make_shared<AddVbkEndorsement>(popminer->btc(), popminer->vbk(), e);

  // before cmd execution we have 0 endorsements
  ASSERT_EQ(vbk5->getContainingEndorsements().size(), 0);
  ASSERT_EQ(vbk5->getEndorsedBy().size(), 0);
  ASSERT_EQ(vbk10->getContainingEndorsements().size(), 0);
  ASSERT_EQ(vbk10->getEndorsedBy().size(), 0);

  // execute command
  ASSERT_TRUE(cmd->Execute(state)) << state.toString();

  // verify that state has been changed
  ASSERT_EQ(vbk5->getContainingEndorsements().size(), 0);
  ASSERT_EQ(vbk5->getEndorsedBy().size(), 1);
  ASSERT_EQ(vbk10->getContainingEndorsements().size(), 1);
  ASSERT_EQ(vbk10->getEndorsedBy().size(), 0);

  // execute again
  ASSERT_TRUE(cmd->Execute(state));

  // verify that another endorsement has been added
  ASSERT_EQ(vbk5->getContainingEndorsements().size(), 0);
  ASSERT_EQ(vbk5->getEndorsedBy().size(), 2);
  ASSERT_EQ(vbk10->getContainingEndorsements().size(), 2);
  ASSERT_EQ(vbk10->getEndorsedBy().size(), 0);

  // unexecute command
  ASSERT_NO_FATAL_FAILURE(cmd->UnExecute());

  // endorsement is removed
  ASSERT_EQ(vbk5->getContainingEndorsements().size(), 0);
  ASSERT_EQ(vbk5->getEndorsedBy().size(), 1);
  ASSERT_EQ(vbk10->getContainingEndorsements().size(), 1);
  ASSERT_EQ(vbk10->getEndorsedBy().size(), 0);

  // unexecute command
  ASSERT_NO_FATAL_FAILURE(cmd->UnExecute());

  // endorsement is removed
  ASSERT_EQ(vbk5->getContainingEndorsements().size(), 0);
  ASSERT_EQ(vbk5->getEndorsedBy().size(), 0);
  ASSERT_EQ(vbk10->getContainingEndorsements().size(), 0);
  ASSERT_EQ(vbk10->getEndorsedBy().size(), 0);
}

TEST_F(AtomicityTestFixture, AddAltEndorsement) {
  popminer->mineBtcBlocks(10);
  popminer->mineVbkBlocks(10);
  std::vector<AltBlock> chain{altparam.getBootstrapBlock()};
  mineAltBlocks(10, chain);

  // alt tip = last alt block in "chain"
  ASSERT_TRUE(alttree.setState(chain.back().getHash(), state));

  // endorsed
  auto alt5 = alttree.getBestChain().tip()->getAncestor(5);
  ASSERT_TRUE(alt5);
  // containing
  auto alt10 = alttree.getBestChain().tip()->getAncestor(10);
  ASSERT_TRUE(alt10);

  auto e = std::make_shared<AltEndorsement>();
  e->id = uint256::fromHex("1");
  e->blockOfProof = popminer->vbk().getBestChain().tip()->getHash();
  e->endorsedHash = alt5->getHash();
  e->containingHash = alt10->getHash();

  auto cmd = std::make_shared<AddAltEndorsement>(popminer->vbk(), alttree, e);

  // before cmd execution we have 0 endorsements
  ASSERT_EQ(alt5->getContainingEndorsements().size(), 0);
  ASSERT_EQ(alt5->getEndorsedBy().size(), 0);
  ASSERT_EQ(alt10->getContainingEndorsements().size(), 0);
  ASSERT_EQ(alt10->getEndorsedBy().size(), 0);

  // execute command
  ASSERT_TRUE(cmd->Execute(state)) << state.toString();

  // verify that state has been changed
  ASSERT_EQ(alt5->getContainingEndorsements().size(), 0);
  ASSERT_EQ(alt5->getEndorsedBy().size(), 1);
  ASSERT_EQ(alt10->getContainingEndorsements().size(), 1);
  ASSERT_EQ(alt10->getEndorsedBy().size(), 0);

  // execute command second time
  ASSERT_TRUE(cmd->Execute(state));

  // verify that state has been changed
  // as duplicates are filtered by addPayloads
  ASSERT_EQ(alt5->getContainingEndorsements().size(), 0);
  ASSERT_EQ(alt5->getEndorsedBy().size(), 2);
  ASSERT_EQ(alt10->getContainingEndorsements().size(), 2);
  ASSERT_EQ(alt10->getEndorsedBy().size(), 0);

  // unexecute command
  ASSERT_NO_FATAL_FAILURE(cmd->UnExecute());

  // endorsement is removed
  ASSERT_EQ(alt5->getContainingEndorsements().size(), 0);
  ASSERT_EQ(alt5->getEndorsedBy().size(), 1);
  ASSERT_EQ(alt10->getContainingEndorsements().size(), 1);
  ASSERT_EQ(alt10->getEndorsedBy().size(), 0);

  // unexecute command
  ASSERT_NO_FATAL_FAILURE(cmd->UnExecute());

  // endorsement is removed
  ASSERT_EQ(alt5->getContainingEndorsements().size(), 0);
  ASSERT_EQ(alt5->getEndorsedBy().size(), 0);
  ASSERT_EQ(alt10->getContainingEndorsements().size(), 0);
  ASSERT_EQ(alt10->getEndorsedBy().size(), 0);

  ASSERT_DEATH(cmd->UnExecute(), "");
}

TEST_F(AtomicityTestFixture, AddVTB) {
  popminer->mineBtcBlocks(10);
  popminer->mineVbkBlocks(10);

  // create a single VTB which has endorsed VBK5, containing - VBK11
  // endorsed
  auto vbk5 = popminer->vbk().getBestChain().tip()->getAncestor(5);
  ASSERT_TRUE(vbk5);
  auto vbkpoptx1 = popminer->createVbkPopTxEndorsingVbkBlock(
      vbk5->getHeader(), getLastKnownBtcBlock());
  auto vbkpoptx2 = popminer->createVbkPopTxEndorsingVbkBlock(
      vbk5->getHeader(), getLastKnownBtcBlock());
  auto vbkcontaining = popminer->mineVbkBlocks(1, {vbkpoptx1, vbkpoptx2});

  // now we have 2 valid VTBs endorsing VBK5
  VTB vtb1 = popminer->createVTB(vbkcontaining->getHeader(), vbkpoptx1);
  VTB vtb2 = popminer->createVTB(vbkcontaining->getHeader(), vbkpoptx2);

  PopData pd;
  pd.vtbs = {vtb1, vtb2};
  payloadsProvider.writePayloads(pd);

  auto cmd1 = std::make_shared<AddVTB>(alttree, vtb1);

  // before execution, we have to supply VBK blocks context to ALT tree...
  // bypass all AltPayloads, and add them directly
  for (auto* block : popminer->vbk().getBestChain()) {
    if (block->pprev == nullptr) {
      // skip genesis block
      continue;
    }

    ASSERT_TRUE(alttree.vbk().acceptBlockHeader(block->getHeader(), state));
  }

  // verify VBK tree does not know about this VTB
  auto altvbkcontaining =
      alttree.vbk().getBlockIndex(vtb1.containingBlock.getHash());
  ASSERT_TRUE(altvbkcontaining);

  auto& vtbids1 = altvbkcontaining->template getPayloadIds<VTB>();
  ASSERT_EQ(vtbids1.size(), 0);

  // execute that AddVTB1
  ASSERT_TRUE(cmd1->Execute(state)) << state.toString();

  {
    // special case: same as cmd1, but different endorsement address. unapply
    // should not fail on assert.
    auto cmd1b = std::make_shared<AddVTB>(alttree, vtb1);
    ASSERT_NO_FATAL_FAILURE(cmd1b->UnExecute());
    ASSERT_TRUE(cmd1b->Execute(state)) << state.toString();
  }

  auto& vtbids2 = altvbkcontaining->template getPayloadIds<VTB>();
  ASSERT_EQ(vtbids2.size(), 1);
  ASSERT_EQ(vtbids2.at(0), vtb1.getId());

  // run execute second time on same VTB
  ASSERT_TRUE(cmd1->Execute(state));
  auto& vtbids3 = altvbkcontaining->template getPayloadIds<VTB>();

  ASSERT_EQ(vtbids3.size(), 1);
  ASSERT_EQ(vtbids3.at(0), vtb1.getId());

  {  // roll back the VTB
    ASSERT_NO_FATAL_FAILURE(cmd1->UnExecute());
    auto vtbids = altvbkcontaining->template getPayloadIds<VTB>();
    ASSERT_EQ(vtbids.size(), 0);

    // return the state
    ASSERT_TRUE(cmd1->Execute(state));
    vtbids = altvbkcontaining->template getPayloadIds<VTB>();

    ASSERT_EQ(vtbids.size(), 1);
    ASSERT_EQ(vtbids.at(0), vtb1.getId());
  }

  // add vtb2
  auto cmd2 = std::make_shared<AddVTB>(alttree, vtb2);
  ASSERT_TRUE(cmd2->Execute(state));
  auto& vtbids4 = altvbkcontaining->template getPayloadIds<VTB>();

  ASSERT_EQ(vtbids4.size(), 2);
  ASSERT_EQ(vtbids4.at(0), vtb1.getId());
  ASSERT_EQ(vtbids4.at(1), vtb2.getId());

  // unexecute VTB2
  ASSERT_NO_FATAL_FAILURE(cmd2->UnExecute());
  auto& vtbids5 = altvbkcontaining->template getPayloadIds<VTB>();

  ASSERT_EQ(vtbids5.size(), 1);
  ASSERT_EQ(vtbids5.at(0), vtb1.getId());

  // unexecute VTB1
  ASSERT_NO_FATAL_FAILURE(cmd1->UnExecute());
  auto& vtbids6 = altvbkcontaining->template getPayloadIds<VTB>();

  ASSERT_EQ(vtbids6.size(), 0);
}
