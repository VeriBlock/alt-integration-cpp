// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_KEYSTONE_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_KEYSTONE_UTIL_HPP_

#include <stdexcept>
#include <veriblock/consts.hpp>

namespace altintegration {

bool isKeystone(int32_t blockNumber, uint32_t keystoneInterval);

int32_t blockHeightToKeystoneNumber(int32_t blockHeight, uint32_t keystoneInterval);

int32_t highestKeystoneAtOrBefore(int32_t blockNumber,
                                  uint32_t keystoneInterval);
int32_t firstKeystoneAfter(int32_t blockNumber, uint32_t keystoneInterval);
int32_t highestBlockWhichConnectsKeystoneToPrevious(
    int32_t blockNumberOfKeystone, uint32_t keystoneInterval);

bool isCrossedKeystoneBoundary(int32_t bottomHeight,
                               int32_t tipHeight,
                               uint32_t keystoneInterval);

bool areOnSameKeystoneInterval(int32_t height1,
                               int32_t height2,
                               uint32_t keystoneInterval);

int32_t getFirstPreviousKeystoneHeight(int32_t height,
                                       uint32_t keystoneInterval);
int32_t getSecondPreviousKeystoneHeight(int32_t height,
                                        uint32_t keystoneInterval);
}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_KEYSTONE_UTIL_HPP_
