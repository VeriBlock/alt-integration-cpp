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
    storedBlock.hash = getExtendedHash(_block.getHash());
    storedBlock.height = _block.height;
    storedBlock.block = _block;
    return storedBlock;
  }

  std::string toRaw() const {
    WriteStream stream;
    block.toRaw(stream);
    return std::string(reinterpret_cast<const char*>(stream.data().data()),
                       stream.data().size());
  }

  static StoredVbkBlock fromRaw(const std::string& bytes) {
    ReadStream stream(bytes);
    VbkBlock block = VbkBlock::fromRaw(stream);
    return fromBlock(block);
  }

  static hash_t getExtendedHash(const uint192& hash) { return ArithUint256(hash); }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_STORED_VBKBLOCK_HPP_
