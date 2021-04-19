// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_KEYSTONE_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_KEYSTONE_UTIL_HPP_

#include <stdexcept>
#include <veriblock/pop/consts.hpp>

namespace altintegration {

/**
 * Calculates if block with height = `blockNumber` is keystone.
 * @param blockNumber blopck height
 * @param keystoneInterval keystone interval
 * @return true if block is keystone
 */
bool isKeystone(int32_t blockNumber, uint32_t keystoneInterval);

//! @private
int32_t blockHeightToKeystoneNumber(int32_t blockHeight,
                                    uint32_t keystoneInterval);

//! @private
int32_t highestKeystoneAtOrBefore(int32_t blockNumber,
                                  uint32_t keystoneInterval);
//! @private
int32_t firstKeystoneAfter(int32_t blockNumber, uint32_t keystoneInterval);

//! @private
int32_t highestBlockWhichConnectsKeystoneToPrevious(
    int32_t blockNumberOfKeystone, uint32_t keystoneInterval);

//! @private
bool isCrossedKeystoneBoundary(int32_t bottomHeight,
                               int32_t tipHeight,
                               uint32_t keystoneInterval);

//! @private
bool areOnSameKeystoneInterval(int32_t height1,
                               int32_t height2,
                               uint32_t keystoneInterval);

//! @private
int32_t getFirstPreviousKeystoneHeight(int32_t height,
                                       uint32_t keystoneInterval);
//! @private
int32_t getSecondPreviousKeystoneHeight(int32_t height,
                                        uint32_t keystoneInterval);
}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_KEYSTONE_UTIL_HPP_
