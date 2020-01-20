#ifndef __SHAUTIL__HPP__
#define __SHAUTIL__HPP__

#include <cstddef>

#include "entities/hashes.hpp"
#include "slice.hpp"

#include "sha256.h"

namespace VeriBlock {

/**
 * Calculates SHA256 of the input data
 * @param data read data from this array
 * @return vector with SHA256 hash of the input data
 */
Sha256Hash sha256get(Slice<uint8_t> data);

/**
 * Calculates SHA256 of the input data twice
 * @param data read data from this array
 * @return vector with SHA256 hash of the input data
 */
Sha256Hash sha256twice(Slice<uint8_t> data);

}  // namespace VeriBlock

#endif  //__SHAUTIL__HPP__
