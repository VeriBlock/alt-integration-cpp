// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/assert.hpp>
#include <veriblock/pop/keystone_util.hpp>

namespace altintegration {

int32_t highestKeystoneAtOrBefore(int32_t blockNumber,
                                  uint32_t keystoneInterval) {
  VBK_ASSERT_MSG(blockNumber >= 0,
                 "can not be called with a negative block "
                 "number");

  int32_t diff = blockNumber % keystoneInterval;
  return blockNumber - diff;
}

int32_t blockHeightToKeystoneNumber(int32_t blockHeight,
                                    uint32_t keystoneInterval) {
  return blockHeight / keystoneInterval;
}

bool isKeystone(int32_t blockNumber, uint32_t keystoneInterval) {
  VBK_ASSERT_MSG(blockNumber >= 0,
                 "can not be called with a negative block "
                 "number");
  return blockNumber % keystoneInterval == 0;
}

int32_t firstKeystoneAfter(int32_t blockNumber, uint32_t keystoneInterval) {
  VBK_ASSERT_MSG(blockNumber >= 0,
                 "can not be called with a negative block "
                 "number");

  if (isKeystone(blockNumber, keystoneInterval)) {
    return blockNumber + keystoneInterval;
  }
  int32_t diff = blockNumber % keystoneInterval;
  return blockNumber + (keystoneInterval - diff);
}

int32_t highestBlockWhichConnectsKeystoneToPrevious(
    int32_t blockNumberOfKeystone, uint32_t keystoneInterval) {
  VBK_ASSERT_MSG(
      isKeystone(blockNumberOfKeystone, keystoneInterval),
      "highestBlockWhichConnectsKeystoneToPrevious() can not be called with "
      "a non-keystone block");

  return blockNumberOfKeystone + keystoneInterval + 1;
}

bool isCrossedKeystoneBoundary(int32_t bottomHeight,
                               int32_t tipHeight,
                               uint32_t keystoneInterval) {
  VBK_ASSERT_MSG(bottomHeight >= 0,
                 "can not be called with a negative bottomHeight");

  VBK_ASSERT_MSG(tipHeight >= 0, "can not be called with a negative tipHeight");

  int32_t keystoneIntervalAmount = bottomHeight / keystoneInterval;
  int32_t tipIntervalAmount = tipHeight / keystoneInterval;

  return keystoneIntervalAmount < tipIntervalAmount;
}

bool areOnSameKeystoneInterval(int32_t height1,
                               int32_t height2,
                               uint32_t keystoneInterval) {
  return (height1 / keystoneInterval) == (height2 / keystoneInterval);
}

int32_t getPreviousKeystoneHeight(int32_t height,
                                  uint32_t keystoneInterval,
                                  uint32_t keystone_num) {
  if ((uint32_t)height <= 1 + keystone_num * keystoneInterval) {
    return 0;
  }

  int32_t ret =
      highestKeystoneAtOrBefore(
          isKeystone(height - 1, keystoneInterval) ? height - 2 : height - 1,
          keystoneInterval) -
      keystone_num * keystoneInterval;
  return ret < 0 ? 0 : ret;
}

}  // namespace altintegration
