#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_STORED_BTCBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_STORED_BTCBLOCK_HPP_

#include <cstdint>

#include "veriblock/entities/btcblock.hpp"
#include "veriblock/uint.hpp"

namespace VeriBlock {

/**
 * @class StoredBtcBlock
 *
 * @brief Represents a BTC block stored in the BlockRepository
 *
 * @invariant block contains fully populated BtcBlock structure.
 * hash, height and block are set before using StoredBtcBlock in
 * other methods.
 */
struct StoredBtcBlock {
  using hash_t = uint256;
  using height_t = uint32_t;

  hash_t hash{};
  height_t height{};
  BtcBlock block{};

  static StoredBtcBlock fromBlock(BtcBlock _block, height_t _height) {
    StoredBtcBlock storedBlock{};
    storedBlock.hash = _block.getHash();
    storedBlock.height = _height;
    storedBlock.block = _block;
    return storedBlock;
  }

  std::string toRaw() const {
    WriteStream stream;
    block.toRaw(stream);
    stream.writeLE<height_t>(height);
    return std::string(reinterpret_cast<const char *>(stream.data().data()), stream.data().size());
  }

  static StoredBtcBlock fromRaw(const std::string& bytes) {
    ReadStream stream(bytes);
    BtcBlock block = BtcBlock::fromRaw(stream);
    height_t _height = stream.readLE<height_t>();
    return fromBlock(block, _height);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_STORED_BTCBLOCK_HPP_
