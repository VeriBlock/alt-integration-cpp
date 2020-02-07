#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_

#include <vector>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/write_stream.hpp"

namespace VeriBlock {

//! Store block
template <typename Block>
struct BlockIndex {
  using hash_t = typename Block::hash_t;
  using height_t = typename Block::height_t;

  //! (memory only) pointer to a previous block
  BlockIndex* pprev;

  //! (memory only) total amount of work in the chain up to and including this
  //! block
  ArithUint256 chainWork = 0;

  //! height of the entry in the chain
  height_t height = 0;

  //! block header
  Block header{};

  void toRaw(WriteStream& stream) {
    stream.writeBE<uint32_t>(height);
    header.toRaw(stream);
  }

  static BlockIndex<Block> fromRaw(ReadStream& stream) {
    BlockIndex<Block> index{};
    index.height = stream.readBE<uint32_t>();
    index.header = Block::fromRaw(stream);
    return index;
  }
};

}  // namespace VeriBlock
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
