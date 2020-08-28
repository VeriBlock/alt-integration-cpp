// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/keystone_util.hpp>

namespace altintegration {

int highestKeystoneAtOrBefore(int blockNumber, int keystoneInterval) {
  if (blockNumber < 0) {
    throw std::invalid_argument(
        "highestKeystoneAtOrBefore can not be called with a negative block "
        "number");
  }

  auto diff = blockNumber % keystoneInterval;
  return blockNumber - diff;
}

bool isKeystone(int blockNumber, int keystoneInterval) {
  if (blockNumber < 0) {
    throw std::invalid_argument("isKeystone(): negative block number");
  }
  return blockNumber % keystoneInterval == 0;
}

int firstKeystoneAfter(int blockNumber, int keystoneInterval) {
  if (blockNumber < 0) {
    throw std::invalid_argument(
        "firstKeystoneAfter can not be called with negative block number");
  }

  if (isKeystone(blockNumber, keystoneInterval)) {
    return blockNumber + keystoneInterval;
  } else {
    auto diff = blockNumber % keystoneInterval;
    return blockNumber + (keystoneInterval - diff);
  }
}

int highestBlockWhichConnectsKeystoneToPrevious(int blockNumberOfKeystone,
                                                int keystoneInterval) {
  if (!isKeystone(blockNumberOfKeystone, keystoneInterval)) {
    throw std::invalid_argument(
        "highestBlockWhichConnectsKeystoneToPrevious() can not be called with "
        "a non-keystone block");
  }

  return blockNumberOfKeystone + keystoneInterval + 1;
}

bool isCrossedKeystoneBoundary(int bottomHeight,
                               int tipHeight,
                               int keystoneInterval) {
  auto keystoneIntervalAmount = bottomHeight / keystoneInterval;
  auto tipIntervalAmount = tipHeight / keystoneInterval;

  return keystoneIntervalAmount < tipIntervalAmount;
}

bool areOnSameKeystoneInterval(int32_t height1,
                               int32_t height2,
                               int32_t keystoneInterval) {
  return (height1 / keystoneInterval) == (height2 / keystoneInterval);
}
}  // namespace altintegration
