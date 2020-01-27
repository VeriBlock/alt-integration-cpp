#ifndef ALT_INTEGRATION_CHECKS_HPP
#define ALT_INTEGRATION_CHECKS_HPP

#include <stdexcept>

/**
 * Contains checks for range, limits etc
 */

namespace VeriBlock {

/**
 * Checks if expression 'min' <= 'num' <= 'max' is true. If false, throws.
 * @param num number to check
 * @param min min value
 * @param max max value
 * @throws std::out_of_range
 */
void checkRange(int64_t num, int64_t min, int64_t max);

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_CHECKS_HPP
