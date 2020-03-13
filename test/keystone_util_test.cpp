#include <gtest/gtest.h>

#include <veriblock/keystone_util.hpp>

using namespace VeriBlock;

const static uint32_t VBK_KEYSTONE_INTERVAL = 20;

TEST(KeystoneUtil, testHighestKeystoneAtOrBefore) {
  ASSERT_ANY_THROW(highestKeystoneAtOrBefore(-1, VBK_KEYSTONE_INTERVAL));
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
  ASSERT_ANY_THROW(isKeystone(-1, VBK_KEYSTONE_INTERVAL));
  ASSERT_TRUE(isKeystone(0, VBK_KEYSTONE_INTERVAL));
  ASSERT_TRUE(isKeystone(20, VBK_KEYSTONE_INTERVAL));
  ASSERT_TRUE(isKeystone(40, VBK_KEYSTONE_INTERVAL));
  ASSERT_FALSE(isKeystone(1, VBK_KEYSTONE_INTERVAL));
  ASSERT_FALSE(isKeystone(21, VBK_KEYSTONE_INTERVAL));
}

TEST(KeystoneUtil, testFirstKeystoneAfter) {
  ASSERT_ANY_THROW(firstKeystoneAfter(-1, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(20, firstKeystoneAfter(0, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(20, firstKeystoneAfter(1, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(20, firstKeystoneAfter(19, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(40, firstKeystoneAfter(20, VBK_KEYSTONE_INTERVAL));
}

TEST(KeystoneUtil, testHighestBlockWhichConnectsKeystoneToPrevious) {
  ASSERT_ANY_THROW(
      highestBlockWhichConnectsKeystoneToPrevious(-1, VBK_KEYSTONE_INTERVAL));
  ASSERT_ANY_THROW(
      highestBlockWhichConnectsKeystoneToPrevious(3, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(
      21,
      highestBlockWhichConnectsKeystoneToPrevious(0, VBK_KEYSTONE_INTERVAL));
  ASSERT_EQ(
      41,
      highestBlockWhichConnectsKeystoneToPrevious(20, VBK_KEYSTONE_INTERVAL));
}
