#ifndef ALT_INTEGRATION_VERIBLOCK_SHAUTIL_HPP
#define ALT_INTEGRATION_VERIBLOCK_SHAUTIL_HPP

#include <cstddef>

#include "entities/hashes.hpp"
#include "sha256.h"
#include "slice.hpp"
#include "vblake.h"

namespace VeriBlock {

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
 * @return vector with SHA256 hash of the input data
 */
uint192 vblake(Slice<const uint8_t> data);

}  // namespace VeriBlock

#endif  //__SHAUTIL__HPP__
