#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_

#include <memory>
#include <unordered_map>
#include <vector>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/endorsements.hpp"
#include "veriblock/entities/payloads.hpp"
#include "veriblock/write_stream.hpp"

namespace altintegration {

//! Store block
template <typename Block>
struct BlockIndex {
  using block_t = Block;
  using hash_t = typename block_t::hash_t;
  using height_t = typename block_t::height_t;
  using endorsement_t = typename block_t::endorsement_t;
  using eid_t = typename endorsement_t::id_t;
  using payloads_t = typename Block::payloads_t;
  using pid_t = typename payloads_t::id_t;

  //! (memory only) pointer to a previous block
  BlockIndex* pprev;

  //! (memory only) total amount of work in the chain up to and including this
  //! block
  ArithUint256 chainWork = 0;

  //! (memory only) list of endorsements that containing in this block
  std::unordered_map<eid_t, std::shared_ptr<endorsement_t>>
      containingEndorsements;

  //! (memory only) list of containing payloads in this block
  std::vector<payloads_id_t> containingPayloads;

  //! height of the entry in the chain
  height_t height = 0;

  std::vector<pid_t> stored_payloads_ids;

  //! block header
  Block header{};

  hash_t getHash() const { return header.getHash(); }
  uint32_t getBlockTime() const { return header.getBlockTime(); }
  uint32_t getDifficulty() const { return header.getDifficulty(); }

  const BlockIndex* getAncestorBlocksBehind(height_t steps) const {
    if (steps < 0 || steps > this->height + 1) {
      return nullptr;
    }

    return this->getAncestor(this->height + 1 - steps);
  }

  const BlockIndex* getAncestor(height_t _height) const {
    if (_height < 0 || _height > this->height) {
      return nullptr;
    }

    // TODO: this algorithm is not optimal. for O(n) seek backwards until we hit
    // valid height. also it assumes whole blockchain is in memory (pprev is
    // valid until given height)
    const BlockIndex* index = this;
    while (index != nullptr) {
      if (index->height > _height) {
        index = index->pprev;
      } else if (index->height == _height) {
        return index;
      } else {
        return nullptr;
      }
    }

    return nullptr;
  }

  void toRaw(WriteStream& stream) const {
    stream.writeBE<uint32_t>(height);
    header.toRaw(stream);
  }

  std::vector<uint8_t> toRaw() const {
    WriteStream stream;
    toRaw(stream);
    return stream.data();
  }

  static BlockIndex fromRaw(ReadStream& stream) {
    BlockIndex index{};
    index.height = stream.readBE<uint32_t>();
    index.header = Block::fromRaw(stream);
    return index;
  }

  static BlockIndex fromRaw(const std::string& bytes) {
    ReadStream stream(bytes);
    return fromRaw(stream);
  }

  friend bool operator==(const BlockIndex& a, const BlockIndex& b) {
    return a.header == b.header;
  }
};

}  // namespace altintegration
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
