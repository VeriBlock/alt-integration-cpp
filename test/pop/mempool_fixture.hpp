#ifndef VERIBLOCK_POP_CPP_MEMPOOL_FIXTURE_HPP
#define VERIBLOCK_POP_CPP_MEMPOOL_FIXTURE_HPP

#include <veriblock/pop/mempool.hpp>

#include <gtest/gtest.h>

#include <vector>
#include <veriblock/pop/alt-util.hpp>

#include "util/pop_test_fixture.hpp"
#include "util/test_utils.hpp"
#include <veriblock/pop/hashutil.hpp>

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
    ASSERT_TRUE(alttree.setState(containingBlock.getHash(), state));
    ASSERT_TRUE(state.IsValid());
    validateAlttreeIndexState(alttree, containingBlock, pop);
  }

  PopData checkedGetPop() {
    auto before = alttree.toPrettyString();
    auto ret = mempool->generatePopData();
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
    mempool->submit(atv, state);
    if (!state.IsValid()) {
      ASSERT_GE(state.GetPath().size(), 12);
      std::string error = state.GetPath();
      error = std::string{error.begin(), error.begin() + 12};
      EXPECT_EQ(error, "atv-stateful");
      state.reset();
    }
  }

  void submitVTB(const VTB& vtb) {
    mempool->submit(vtb, state);
    if (!state.IsValid()) {
      ASSERT_GE(state.GetPath().size(), 12);
      std::string error = state.GetPath();
      error = std::string{error.begin(), error.begin() + 12};
      EXPECT_EQ(error, "vtb-stateful");
      state.reset();
    }
  }

  void submitVBK(const VbkBlock& vbk) {
    mempool->submit(vbk, state);
    if (!state.IsValid()) {
      ASSERT_GE(state.GetPath().size(), 12);
      std::string error = state.GetPath();
      error = std::string{error.begin(), error.begin() + 12};
      EXPECT_EQ(error, "vbk-stateful");
      state.reset();
    }
  }
};

#endif  // VERIBLOCK_POP_CPP_MEMPOOL_FIXTURE_HPP
