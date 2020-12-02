// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/alt-util.hpp>
#include <veriblock/mock_miner.hpp>

using namespace altintegration;

struct AltUtilTest : public ::testing::Test {
  MockMiner popminer;
};

TEST_F(AltUtilTest, GetLastKnownBlocks) {
  auto genesis = getLastKnownBlocks(popminer.btc(), 1ull);
  ASSERT_EQ(genesis.size(), 1);
  ASSERT_EQ(genesis[0], GetRegTestBtcBlock().getHash().asVector());

  auto a = popminer.mineBtcBlocks(10);
  auto one = getLastKnownBlocks(popminer.btc(), 1);
  ASSERT_EQ(one.size(), 1);
  ASSERT_EQ(one[0], a->getHash().asVector());

  auto ten = getLastKnownBlocks(popminer.btc(), 10ull);
  ASSERT_EQ(ten.size(), 10);
  for (size_t i = 0; i < 10; i++) {
    ASSERT_EQ(ten[i], a->getAncestor((int)(1 + i))->getHash().asVector());
  }

  auto many = getLastKnownBlocks(popminer.btc(), 1000ull);
  ASSERT_EQ(many.size(), 11ull);  // we know only 11 bocks, 10+genesis
}

TEST_F(AltUtilTest, CalculateContextInfoContainerHash) {
  PopData empty;
  empty.version = 2;
  auto mroot2 = empty.getMerkleRoot();
  empty.version = 1;
  auto mroot1 = empty.getMerkleRoot();

  EXPECT_EQ(HexStr(mroot2),
            "2bb64e7e70e4596fd0db265a75a3537079f0d77aaf0935aee6379b004db8ff9d");
  EXPECT_EQ(HexStr(mroot1), "eaac496a5eab315c9255fb85c871cef7fd87047adcd2e81ba7d55d6bdeb1737f");

  auto hash0 = CalculateContextInfoContainerHash(empty, nullptr, 5, 0);
  EXPECT_EQ(HexStr(hash0), "73511044b1dc34f9d920e8fb85ab1a1b10de4e31dc6272ce23386fe0d8b5b600");

  auto hash1 = CalculateContextInfoContainerHash(empty, nullptr, 5, 1);
  EXPECT_EQ(HexStr(hash1), "6cca10728ad80e77554b41850c4b22d1016d75e8eca3d0916da916eb760fc350");
}