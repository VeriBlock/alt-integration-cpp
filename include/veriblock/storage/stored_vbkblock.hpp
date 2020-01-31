#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_STORED_VBKBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_STORED_VBKBLOCK_HPP_

#include <cstdint>

#include "veriblock/uint.hpp"
#include "veriblock/arith_uint256.hpp"

#include "veriblock/entities/vbkblock.hpp"

namespace VeriBlock {

/**
 * @class StoredVbkBlock
 *
 * @brief Represents a VBK block stored in the BlockRepository
 *
 * @invariant block contains fully populated VbkBlock structure.
 * hash, height and block are set before using StoredVbkBlock in
 * other methods.
 */
struct StoredVbkBlock {
  using hash_t = uint256;
  using height_t = uint32_t;

  hash_t hash{};
  height_t height{};
  VbkBlock block{};

  static StoredVbkBlock fromBlock(VbkBlock _block) {
    StoredVbkBlock storedBlock{};
    uint192 hash = _block.getHash();
    ///TODO: I think it is better to prepend zeroes than append
    ArithUint256 extendedHash(hash);
    storedBlock.hash = extendedHash;
    storedBlock.height = _block.height;
    storedBlock.block = _block;
    return storedBlock;
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_STORED_VBKBLOCK_HPP_
