// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_TIME_HPP
#define ALT_INTEGRATION_VERIBLOCK_TIME_HPP

#include <time.h>

namespace altintegration {

inline uint32_t currentTimestamp4() { return (uint32_t)time(0); }

}  // namespace altintegration

#endif  // ! ALT_INTEGRATION_INCLUDE_VERIBLOCK
