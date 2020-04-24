#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>
#include <veriblock/mock_miner.hpp>

using namespace altintegration;

struct AltPayloadsAtomic : public ::testing::Test, public PopTestFixture {
  BlockIndex<BtcBlock>* btcTip = popminer.mineBtcBlocks(100);
  BlockIndex<VbkBlock>* vbkTip = popminer.mineVbkBlocks(100);

  AltTree copy = makeEmptyAltTree();
};

namespace altintegration {

void PrintTo(const AltTree::block_index_t& b, std::ostream* os) {
  *os << "block index size: " << b.size() << "\n";
  for (auto& p : b) {
    *os << HexStr(p.first).substr(0, 8) << ": " << p.second->toPrettyString()
        << "\n";
  }
}

}  // namespace altintegration

TEST_F(AltPayloadsAtomic, AddPayloads) {
  std::vector<AltBlock> chain{alttree.getParams().getBootstrapBlock()};
  mineAltBlocks(3, chain);
  // duplicate blocks in 'copy' tree
  for (auto& b : chain) {
    ASSERT_TRUE(copy.acceptBlock(b, state));
  }

  auto& endorsed = chain[chain.size() - 2];
  auto& containing = chain[chain.size() - 1];

  // 3 valid payloads
  auto pv1 = endorseAltBlock(endorsed, containing);
  auto pv2 = endorseAltBlock(endorsed, containing);
  auto pv3 = endorseAltBlock(endorsed, containing);
  // 1 invalid payloads
  auto pinv = endorseAltBlock(endorsed, containing);
  pinv.atv.containingBlock.previousBlock = std::vector<uint8_t>{1, 1, 1, 1};

  // do addPayloads
  ASSERT_FALSE(alttree.addPayloads(containing, {pv1, pv2, pv3, pinv}, state));
  ASSERT_EQ(state.GetPath(),
            "bad-altpayloads+3+bad-atv-containing-block+bad-prev-block");
  state.clear();

  // expect exactly same error for same inputs
  ASSERT_FALSE(alttree.addPayloads(containing, {pv1, pv2, pv3, pinv}, state));
  ASSERT_EQ(state.GetPath(),
            "bad-altpayloads+3+bad-atv-containing-block+bad-prev-block");
  state.clear();

  // if we remove invalid payloads, expect to successfully add payloads
  ASSERT_TRUE(alttree.addPayloads(containing, {pv1, pv2, pv3}, state));

  // add same payloads to a `copy`
  ASSERT_TRUE(copy.addPayloads(containing, {pv1, pv2, pv3}, state));

  // expect that trees are equal
  EXPECT_EQ(alttree.toPrettyString(), copy.toPrettyString());
  EXPECT_EQ(alttree.btc(), copy.btc());
  EXPECT_EQ(alttree.vbk(), copy.vbk());
  EXPECT_EQ(alttree.getComparator(), copy.getComparator());
  EXPECT_EQ(alttree, copy);
}