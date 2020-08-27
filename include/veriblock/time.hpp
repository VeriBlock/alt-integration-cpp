// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_TIME_HPP
#define ALT_INTEGRATION_VERIBLOCK_TIME_HPP

#include <time.h>

#include <cstdint>

namespace altintegration {

/**
 * Set mock time for usage in unit tests. 0 disables mock time.
 * @param mocktime
 */
void setMockTime(uint32_t mocktime);

//! Get current mock time.
uint32_t getMockTime();

//! Get current time as 4 bytes. If mock time is set, returns mock time.
uint32_t currentTimestamp4();

}  // namespace altintegration

#endif  // ! ALT_INTEGRATION_INCLUDE_VERIBLOCK
