// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_CHECKS_HPP
#define ALT_INTEGRATION_CHECKS_HPP

#include <stdexcept>

/**
 * Contains checks for range, limits etc
 */

namespace altintegration {

/**
 * Checks if expression 'min' <= 'num' <= 'max' is true. If false, throws.
 * @param num number to check
 * @param min min value
 * @param max max value
 * @throws std::out_of_range
 */
void checkRange(int64_t num, int64_t min, int64_t max);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_CHECKS_HPP
