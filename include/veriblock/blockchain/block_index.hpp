// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_

#include <memory>
#include <set>
#include <vector>
#include <veriblock/arith_uint256.hpp>
#include <veriblock/blockchain/command.hpp>
#include <veriblock/blockchain/command_group.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/validation_state.hpp>
#include <veriblock/write_stream.hpp>

namespace altintegration {

enum BlockStatus : uint32_t {
  //! default state for validity - validity state is unknown
  BLOCK_VALID_UNKNOWN = 0,
  //! acceptBlock succeded. All ancestors are at least at this state.
  BLOCK_VALID_TREE = 1 << 0,
  //! all validity flags
  BLOCK_VALID_MASK = BLOCK_VALID_TREE,
  //! this is a bootstrap block
  BLOCK_BOOTSTRAP = 1 << 1,
  //! block is statelessly valid, but the altchain marked it as failed
  BLOCK_FAILED_BLOCK = 1 << 2,
  //! block failed state{less,ful} validation due to its payloads
  BLOCK_FAILED_POP = 1 << 3,
  //! block is state{lessly,fully} valid and the altchain did not report it as
  //! invalid, but some of the ancestor blocks are invalid
  BLOCK_FAILED_CHILD = 1 << 4,
  //! all invalidity flags
  BLOCK_FAILED_MASK =
      BLOCK_FAILED_CHILD | BLOCK_FAILED_POP | BLOCK_FAILED_BLOCK,
  //! the block has been applied via PopStateMachine
  BLOCK_APPLIED = 1 << 5,
  //! the block has been at least once applied via PopStateMachine
  BLOCK_ONCE_APPLIED = 1 << 6,
};

/**
 * Check if the `reason` value can be used as the reason for invalidateSubtree
 * and revalidateSubtree
 */
constexpr bool isValidInvalidationReason(const enum BlockStatus reason) {
  return reason == BLOCK_FAILED_BLOCK || reason == BLOCK_FAILED_POP;
}

//! Store block
template <typename Block>
struct BlockIndex : public Block::addon_t {
  using block_t = Block;
  using addon_t = typename Block::addon_t;
  using hash_t = typename block_t::hash_t;
  using prev_hash_t = typename block_t::prev_hash_t;
  using height_t = typename block_t::height_t;

  //! (memory only) pointer to a previous block
  BlockIndex* pprev = nullptr;

  //! (memory only) a set of pointers for forward iteration
  std::set<BlockIndex*> pnext{};

  //! (memory only) contains status flags
  uint32_t status = 0;  // unknown validity

  bool isValid(enum BlockStatus upTo = BLOCK_VALID_TREE) const {
    VBK_ASSERT(!(upTo & ~BLOCK_VALID_MASK));  // Only validity flags allowed.
    if ((status & BLOCK_FAILED_MASK) != 0u) {
      // block failed
      return false;
    }
    return ((status & BLOCK_VALID_MASK) >= upTo);
  }

  void setNull() {
    addon_t::setNull();
    this->pprev = nullptr;
    this->pnext.clear();
    this->height = 0;
    this->status = 0;
    // make it dirty by default
    setDirty();
  }

  bool raiseValidity(enum BlockStatus upTo) {
    VBK_ASSERT(!(upTo & ~BLOCK_VALID_MASK));  // Only validity flags allowed.
    if ((status & BLOCK_FAILED_MASK) != 0u) {
      return false;
    }
    if ((status & BLOCK_VALID_MASK) < upTo) {
      status = (status & ~BLOCK_VALID_MASK) | upTo;
      return true;
    }
    return false;
  }

  void setDirty() { this->dirty = true; }
  void unsetDirty() { this->dirty = false; }
  bool isDirty() const { return this->dirty; }

  void setFlag(enum BlockStatus s) { this->status |= s; }
  void unsetFlag(enum BlockStatus s) { this->status &= ~s; }

  bool hasFlags(enum BlockStatus s) const { return this->status & s; }

  hash_t getHash() const { return header->getHash(); }
  uint32_t getBlockTime() const { return header->getBlockTime(); }
  uint32_t getDifficulty() const { return header->getDifficulty(); }

  height_t getHeight() const { return height; }
  void setHeight(const height_t newHeight) {
    height = newHeight;
    setDirty();
  }

  const block_t& getHeader() const { return *header; }

  void setHeader(const block_t& newHeader) {
    header = std::make_shared<block_t>(newHeader);
    setDirty();
  }

  void setHeader(std::shared_ptr<block_t> newHeader) {
    header = std::move(newHeader);
    setDirty();
  }

  /**
   * The block is a valid tip if it is valid and either there are no descendant
   * blocks or all the descendants are invalid
   */
  bool isValidTip() const {
    return isValid() &&
           (pnext.empty() ||
            std::all_of(pnext.begin(), pnext.end(), [](BlockIndex* index) {
              return !index->isValid();
            }));
  }

  /**
   *  Check if all immediate descendants of the block are unapplied
   */
  bool allDescendantsUnapplied() const {
    return pnext.empty() ||
           std::all_of(pnext.begin(), pnext.end(), [](BlockIndex* index) {
             return !index->hasFlags(BLOCK_APPLIED);
           });
  }

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

  std::string toPrettyString(size_t level = 0) const {
    return fmt::sprintf("%s%sBlockIndex{height=%d, hash=%s, status=%d, %s}",
                        std::string(level, ' '),
                        Block::name(),
                        height,
                        HexStr(getHash()),
                        status,
                        addon_t::toPrettyString());
  }

  std::string toShortPrettyString() const {
    return fmt::sprintf("%s:%d:%s", Block::name(), height, HexStr(getHash()));
  }

  void toRaw(WriteStream& stream) const {
    stream.writeBE<uint32_t>(height);
    header->toRaw(stream);
    addon_t::toRaw(stream);
  }

  void initFromRaw(ReadStream& stream) {
    height = stream.readBE<uint32_t>();
    header = std::make_shared<Block>(Block::fromRaw(stream));
    addon_t::initAddonFromRaw(stream);
    setDirty();
  }

  std::vector<uint8_t> toRaw() const {
    WriteStream stream;
    toRaw(stream);
    return stream.data();
  }

  static BlockIndex fromRaw(ReadStream& stream) {
    BlockIndex index{};
    index.initFromRaw(stream);
    return index;
  }

  static BlockIndex fromRaw(Slice<const uint8_t> bytes) {
    ReadStream stream(bytes);
    return fromRaw(stream);
  }

 protected:
  //! height of the entry in the chain
  height_t height = 0;

  //! block header
  std::shared_ptr<block_t> header{};

  //! (memory only) if true, this block should be written on disk
  bool dirty = false;
};

template <typename Block>
void PrintTo(const BlockIndex<Block>& b, ::std::ostream* os) {
  *os << b.toPrettyString();
}

template <typename JsonValue, typename Block>
JsonValue ToJSON(const BlockIndex<Block>& i) {
  auto obj = json::makeEmptyObject<JsonValue>();
  json::putIntKV(obj, "height", i.height);
  json::putKV(obj, "header", ToJSON<JsonValue>(*i.header));
  json::putIntKV(obj, "status", i.status);
  return obj;
}

}  // namespace altintegration
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
