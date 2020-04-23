// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_

#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>
#include <veriblock/validation_state.hpp>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/blockchain/command.hpp"
#include "veriblock/blockchain/command_history.hpp"
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
  using context_t = typename Block::context_t;
  using payloads_t = typename Block::payloads_t;
  using protecting_block_t = typename Block::protecting_block_t;

  //! pointer to a previous block
  BlockIndex* pprev{};

  //! total amount of work in the chain up to and including this
  //! block
  ArithUint256 chainWork = 0;

  //! list of containing endorsements in this block
  std::unordered_map<eid_t, std::shared_ptr<endorsement_t>>
      containingEndorsements{};

  //! list of endorsements pointing to this block
  std::vector<endorsement_t*> endorsedBy{};

  //! changelist that this block introduces
  std::vector<CommandPtr> commands{};

  //! height of the entry in the chain
  height_t height = 0;

  //! block header
  std::shared_ptr<Block> header{};

  hash_t getHash() const { return header->getHash(); }
  uint32_t getBlockTime() const { return header->getBlockTime(); }
  uint32_t getDifficulty() const { return header->getDifficulty(); }

  const BlockIndex* getAncestorBlocksBehind(height_t steps) const {
    if (steps < 0 || steps > this->height + 1) {
      return nullptr;
    }

    return this->getAncestor(this->height + 1 - steps);
  }

  BlockIndex* getAncestor(height_t _height) const {
    if (_height < 0 || _height > this->height) {
      return nullptr;
    }

    // TODO: this algorithm is not optimal. for O(n) seek backwards until we hit
    // valid height. also it assumes whole blockchain is in memory (pprev is
    // valid until given height)
    BlockIndex* index = const_cast<BlockIndex*>(this);
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

  std::string toPrettyString() const {
    return "BlockIndex{height=" + std::to_string(height) +
           ", hash=" + HexStr(getHash()).substr(0, 8) +
           ", endorsedBy=" + std::to_string(endorsedBy.size()) +
           ", containsEndorsements=" +
           std::to_string(containingEndorsements.size()) + "}";
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
    return *a.header == *b.header;
  }

  friend bool operator!=(const BlockIndex& a, const BlockIndex& b) {
    return !operator==(a, b);
  }
};

template <typename Block>
void PrintTo(const BlockIndex<Block>& b, ::std::ostream* os) {
  *os << b.toPrettyString();
}

}  // namespace altintegration
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
