#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>
#include <veriblock/mock_miner.hpp>

using namespace altintegration;

struct AltCrosstreeAtomicityTest : public ::testing::Test,
                                   public PopTestFixture {
  BlockIndex<BtcBlock>* btcTip = popminer.mineBtcBlocks(100);
  BlockIndex<VbkBlock>* vbkTip = popminer.mineVbkBlocks(100);
};

namespace altintegration {

void PrintTo(const AltTree::block_index_t& b, std::ostream* os) {
  *os << "block index size: " << b.size() << "\n";
  for (auto& p : b) {
    *os << HexStr(p.first).substr(0, 8) << ": " << p.second->toPrettyString() << "\n";
  }
}

}  // namespace altintegration

TEST_F(AltCrosstreeAtomicityTest, ValidationOnAcopyThenAddPayloads) {
  std::vector<AltBlock> chain{alttree.getParams().getBootstrapBlock()};
  mineAltBlocks(3, chain);

  auto& endorsed = chain[chain.size() - 2];
  auto& containing = chain[chain.size() - 1];

  // 3 valid payloads
  auto pv1 = endorseAltBlock(endorsed, containing);
  auto pv2 = endorseAltBlock(endorsed, containing);
  auto pv3 = endorseAltBlock(endorsed, containing);

  CommandHistory history;
  // do addPayloads for stateful validation
  ASSERT_TRUE(alttree.addPayloads(containing, {pv1, pv2, pv3}, state, history));

  // rollback changes
  history.undoAll();
  history.clear();

  // expect that all payloads can now be added to alttree
  ASSERT_TRUE(alttree.addPayloads(containing, {pv1, pv2, pv3}, state, history))
      << state.toString();
}