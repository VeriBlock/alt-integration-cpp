// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>
#include <veriblock/pop/bootstraps.hpp>

using namespace altintegration;

TEST(GenesisBlock, Vbk) {
  auto block = GetRegTestVbkBlock();
  ASSERT_EQ(block.getHash().toHex(), "a3e77a18a8f7b4568a062f69340d1ad4360382e5bc218a8c");
}

TEST(GenesisBlock, Btc) {
  auto block = GetRegTestBtcBlock();
  ASSERT_EQ(block.getHash().toHex(), "0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206");
}