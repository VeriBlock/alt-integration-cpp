#include <gtest/gtest.h>

#include "util/literals.hpp"
#include <veriblock/storage/pop_storage.hpp>
#include <util/pop_test_fixture.hpp>
#include <veriblock/alt-util.hpp>

using namespace altintegration;

struct AltTreeRepositoryTest : public ::testing::Test, public PopTestFixture {
  AltTreeRepositoryTest() {
  }
};

TEST_F(AltTreeRepositoryTest, Basic) {
  PopStorage storage{};

  popminer->mineBtcBlocks(2);
  popminer->mineVbkBlocks(2);
  storage.saveBtcTree(popminer->btc());
  storage.saveVbkTree(popminer->vbk());

  BlockTree<BtcBlock, BtcChainParams> reloadedBtcTree{btcparam};
  storage.loadBtcTree(reloadedBtcTree);

  VbkBlockTree reloadedVbkTree{vbkparam, btcparam};
  storage.loadVbkTree(reloadedVbkTree);

  EXPECT_TRUE(reloadedBtcTree == popminer->btc());
  EXPECT_TRUE(reloadedVbkTree == popminer->vbk());
}
