// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/keystone_util.hpp>

using namespace altintegration;

const static uint32_t VBK_KEYSTONE_INTERVAL = 20;

TEST(KeystoneUtil, PrevKeystones) {
  ASSERT_EQ(0, getFirstPreviousKeystoneHeight(0, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(0, getSecondPreviousKeystoneHeight(0, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(0, getFirstPreviousKeystoneHeight(19, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(0, getSecondPreviousKeystoneHeight(19, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(0, getFirstPreviousKeystoneHeight(20, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(0, getSecondPreviousKeystoneHeight(20, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(20, getFirstPreviousKeystoneHeight(25, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(0, getSecondPreviousKeystoneHeight(25, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(40, getFirstPreviousKeystoneHeight(41, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(20, getSecondPreviousKeystoneHeight(41, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(40, getFirstPreviousKeystoneHeight(59, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(20, getSecondPreviousKeystoneHeight(59, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(80, getFirstPreviousKeystoneHeight(100, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(60, getSecondPreviousKeystoneHeight(100, VBK_KEYSTONE_INTERVAL));
}

TEST(KeystoneUtil, testHighestKeystoneAtOrBefore) {
  ASSERT_DEATH(highestKeystoneAtOrBefore(-1, VBK_KEYSTONE_INTERVAL), "");
  ASSERT_EQ(highestKeystoneAtOrBefore(0, VBK_KEYSTONE_INTERVAL), 0);
  ASSERT_EQ(highestKeystoneAtOrBefore(1, VBK_KEYSTONE_INTERVAL), 0);
  ASSERT_EQ(highestKeystoneAtOrBefore(20, VBK_KEYSTONE_INTERVAL), 20);
  ASSERT_EQ(highestKeystoneAtOrBefore(21, VBK_KEYSTONE_INTERVAL), 20);
  ASSERT_EQ(highestKeystoneAtOrBefore(23, VBK_KEYSTONE_INTERVAL), 20);
  ASSERT_EQ(highestKeystoneAtOrBefore(30, VBK_KEYSTONE_INTERVAL), 20);
  ASSERT_EQ(highestKeystoneAtOrBefore(39, VBK_KEYSTONE_INTERVAL), 20);
  ASSERT_EQ(highestKeystoneAtOrBefore(40, VBK_KEYSTONE_INTERVAL), 40);
  ASSERT_EQ(highestKeystoneAtOrBefore(41, VBK_KEYSTONE_INTERVAL), 40);
  ASSERT_EQ(highestKeystoneAtOrBefore(50, VBK_KEYSTONE_INTERVAL), 40);
  ASSERT_EQ(highestKeystoneAtOrBefore(100, VBK_KEYSTONE_INTERVAL), 100);
  ASSERT_EQ(highestKeystoneAtOrBefore(101, VBK_KEYSTONE_INTERVAL), 100);
}

TEST(KeystoneUtil, testIsKeystone) {
  ASSERT_DEATH(isKeystone(-1, VBK_KEYSTONE_INTERVAL), "");
  ASSERT_TRUE(isKeystone(0, VBK_KEYSTONE_INTERVAL));
  ASSERT_TRUE(isKeystone(20, VBK_KEYSTONE_INTERVAL));
  ASSERT_TRUE(isKeystone(40, VBK_KEYSTONE_INTERVAL));
  ASSERT_FALSE(isKeystone(1, VBK_KEYSTONE_INTERVAL));
  ASSERT_FALSE(isKeystone(21, VBK_KEYSTONE_INTERVAL));
}

TEST(KeystoneUtil, testFirstKeystoneAfter) {
  ASSERT_DEATH(firstKeystoneAfter(-1, VBK_KEYSTONE_INTERVAL), "");
  ASSERT_EQ(20, firstKeystoneAfter(0, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(20, firstKeystoneAfter(1, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(20, firstKeystoneAfter(19, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(40, firstKeystoneAfter(20, VBK_KEYSTONE_INTERVAL));
}

TEST(KeystoneUtil, testHighestBlockWhichConnectsKeystoneToPrevious) {
  ASSERT_DEATH(
      highestBlockWhichConnectsKeystoneToPrevious(-1, VBK_KEYSTONE_INTERVAL),
      "");
  ASSERT_DEATH(
      highestBlockWhichConnectsKeystoneToPrevious(3, VBK_KEYSTONE_INTERVAL),
      "");
  ASSERT_EQ(
      21,
      highestBlockWhichConnectsKeystoneToPrevious(0, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(
      41,
      highestBlockWhichConnectsKeystoneToPrevious(20, VBK_KEYSTONE_INTERVAL));
}

TEST(KeystoneUtil, testAreOnSameKeystoneInterval) {
  ASSERT_TRUE(areOnSameKeystoneInterval(1, 2, 5));
  ASSERT_FALSE(areOnSameKeystoneInterval(1, 5, 5));
  ASSERT_TRUE(areOnSameKeystoneInterval(6, 5, 5));
  ASSERT_TRUE(areOnSameKeystoneInterval(22, 21, 20));
  ASSERT_FALSE(areOnSameKeystoneInterval(40, 21, 20));
  ASSERT_TRUE(areOnSameKeystoneInterval(39, 21, 20));
}
