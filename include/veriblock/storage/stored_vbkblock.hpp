#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_STORED_VBKBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_STORED_VBKBLOCK_HPP_

#include <cstdint>

#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/uint.hpp"

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
  using hash_t = VbkBlock::hash_t;
  using height_t = uint32_t;

  hash_t hash{};
  height_t height{};
  VbkBlock block{};

  hash_t getHash() const { return hash; }

  static StoredVbkBlock fromBlock(VbkBlock _block) {
    StoredVbkBlock storedBlock{};
    storedBlock.hash = _block.getHash();
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
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_STORED_VBKBLOCK_HPP_
