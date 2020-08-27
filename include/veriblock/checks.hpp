// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_CHECKS_HPP
#define ALT_INTEGRATION_CHECKS_HPP

#include <stdexcept>
#include <veriblock/validation_state.hpp>

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

/**
 * Checks if expression 'min' <= 'num' <= 'max' is true. If false, returns invalid state with error description.
 * @param num number to check
 * @param min min value
 * @param max max value
 * @param state will return error description here
 * @return true if check is OK, false otherwise
 */
bool checkRange(int64_t num, int64_t min, int64_t max, ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_CHECKS_HPP
