#ifndef ALT_INTEGRATION_VERIBLOCK_SHAUTIL_HPP
#define ALT_INTEGRATION_VERIBLOCK_SHAUTIL_HPP

#include <cstddef>

#include "slice.hpp"
#include "vblake.h"
#include "veriblock/third_party/sha256.h"
#include "veriblock/uint.hpp"

namespace altintegration {

/**
 * Calculates SHA256 of the input data
 * @param data read data from this array
 * @return vector with SHA256 hash of the input data
 */
uint256 sha256(Slice<const uint8_t> data);

/**
 * Calculates SHA256 of the input data twice
 * @param data read data from this array
 * @return vector with SHA256 hash of the input data
 */
uint256 sha256twice(Slice<const uint8_t> data);

/**
 * Calculates VBlake of the input data
 * @param data read data from this array
 * @return vector with VBlake hash of the input data
 */
uint192 vblake(Slice<const uint8_t> data);

}  // namespace altintegration

#endif  //__SHAUTIL__HPP__
