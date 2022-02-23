#ifndef VERIBLOCK_POP_CPP_MEMPOOL_FIXTURE_HPP
#define VERIBLOCK_POP_CPP_MEMPOOL_FIXTURE_HPP

#include <gtest/gtest.h>

#include <vector>
#include <veriblock/pop/alt-util.hpp>
#include <veriblock/pop/hashutil.hpp>
#include <veriblock/pop/mempool.hpp>

#include "pop/util/pop_test_fixture.hpp"
#include "pop/util/test_utils.hpp"

using namespace altintegration;

struct MemPoolFixture : public PopTestFixture, public ::testing::Test {
  std::vector<AltBlock> chain = {altparam.getBootstrapBlock()};

  void applyInNextBlock(const PopData& pop) {
    auto containingBlock = generateNextBlock(chain.back());
    chain.push_back(containingBlock);
    ASSERT_TRUE(alttree.acceptBlockHeader(containingBlock, state));
    ASSERT_TRUE(state.IsValid()) << state.toString();
    ASSERT_TRUE(AddPayloads(containingBlock.getHash(), pop))
        << state.toString();
    ASSERT_TRUE(state.IsValid());
    ASSERT_TRUE(alttree.setState(containingBlock.getHash(), state))
        << state.toString();
    ASSERT_TRUE(state.IsValid());
    validateAlttreeIndexState(alttree, containingBlock, pop);
  }

  PopData checkedGetPop() {
    auto before = alttree.toPrettyString();
    auto ret = mempool.generatePopData();
    auto after = alttree.toPrettyString();
    EXPECT_EQ(before, after);
    return ret;
  }

  void removeLastAltBlock() {
    alttree.removeSubtree(chain.back().getHash());
    chain.pop_back();
    ValidationState dummy;
    alttree.setState(chain.back().getHash(), dummy);
  }

  void submitATV(const ATV& atv) {
    auto res = mempool.submit(atv, state);
    EXPECT_TRUE(res.isAccepted()) << state.toString();
    state.reset();
  }

  void submitVTB(const VTB& vtb) {
    auto res = mempool.submit(vtb, state);
    EXPECT_TRUE(res.isAccepted()) << state.toString();
    state.reset();
  }

  void submitVBK(const VbkBlock& vbk) {
    auto res = mempool.submit(vbk, state);
    EXPECT_TRUE(res.isAccepted()) << state.toString();
    state.reset();
  }
};

#endif  // VERIBLOCK_POP_CPP_MEMPOOL_FIXTURE_HPP
