#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_KEYSTONE_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_KEYSTONE_UTIL_HPP_

#include <stdexcept>
#include <veriblock/consts.hpp>

namespace altintegration {

bool isKeystone(int blockNumber, int keystoneInterval);
int highestKeystoneAtOrBefore(int blockNumber, int keystoneInterval);
int firstKeystoneAfter(int blockNumber, int keystoneInterval);
int highestBlockWhichConnectsKeystoneToPrevious(int blockNumberOfKeystone,
                                                int keystoneInterval);

bool isCrossedKeystoneBoundary(int bottomHeight,
                               int tipHeight,
                               int keystoneInterval);
}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_KEYSTONE_UTIL_HPP_
