// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_

#include <memory>
#include <set>
#include <vector>
#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/arith_uint256.hpp>
#include <veriblock/pop/entities/endorsements.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/logger.hpp>
#include <veriblock/pop/storage/stored_block_index.hpp>
#include <veriblock/pop/validation_state.hpp>
#include <veriblock/pop/write_stream.hpp>

#include "block_status.hpp"
#include "command.hpp"
#include "command_group.hpp"

namespace altintegration {

/**
 * A node in a block tree.
 * @tparam Block
 */
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

  //! (memory only) if true, this block can not be reorganized
  bool finalized = false;

  //! create a regular block in a deleted state
  BlockIndex(BlockIndex* prev) : pprev(prev) {
    // FIXME: prev should be a reference
    VBK_ASSERT(prev != nullptr);

    height = prev->getHeight() + 1;

    // TODO: what dirtiness should it have here?
  }

  //! create a bootstrap block in a deleted state
  explicit BlockIndex(height_t _height) : pprev(nullptr), height(_height) {}

  ~BlockIndex() {
    // make sure we deleted this block from prev->pnext
    if (!isDeleted()) {
      deleteTemporarily();
    }

    // disconnect from next blocks so that next blocks won't have invalid
    // pointers
    for (auto* it : pnext) {
      it->pprev = nullptr;
    }
  }

  // BlockIndex is not copyable
  // BlockIndex is movable
  BlockIndex(BlockIndex&& other) = default;
  BlockIndex& operator=(BlockIndex&& other) = default;

  bool isDeleted() const { return hasFlags(BLOCK_DELETED); };

  void restore() {
    VBK_ASSERT_MSG(isDeleted(),
                   "tried to restore block %s that is not deleted",
                   toPrettyString());

    if (pprev != nullptr) {
      pprev->pnext.insert(this);

      if (!pprev->isValid()) {
        setFlag(BLOCK_FAILED_CHILD);
      }
    }

    // unsetFlag runs setDirty() for us
    unsetFlag(BLOCK_DELETED);
  }

  void deleteTemporarily() {
    VBK_ASSERT_MSG(!isDeleted(),
                   "tried to delete block %s that is already deleted",
                   toPrettyString());

    if (pprev != nullptr) {
      pprev->pnext.erase(this);
    }

    // FIXME: this is a hack that might eventually bite us
    addon_t::setNull();

    this->status = BLOCK_VALID_UNKNOWN | BLOCK_DELETED;
    // make it dirty by default
    setDirty();
  }

  // loads on-disk fields from 'other' block index.
  // works like (explicit) copy constructor
  void mergeFrom(const StoredBlockIndex<Block>& other) {
    setHeight(other.height);
    setHeader(other.header);
    setStatus(other.status);

    other.addon.toInmem(*this);
  }

  StoredBlockIndex<Block> toStoredBlockIndex() const {
    StoredBlockIndex<Block> ret;
    ret.height = height;
    ret.status = status;
    ret.header = header;
    ret.addon = *this;
    return ret;
  }

  /**
   * Block is connected if it contains block body (PopData), and all its
   * ancestors are connected.
   * @return true if block is connected, false otherwise.
   */
  bool isConnected() const noexcept {
    return this->isValidUpTo(BLOCK_CONNECTED);
  }

  uint32_t getStatus() const { return status; }
  void setStatus(uint32_t _status) {
    this->status = _status;
    setDirty();
  }

  uint32_t getValidityLevel() const {
    auto level = status & BLOCK_VALID_MASK;
    VBK_ASSERT_MSG(level <= BLOCK_CAN_BE_APPLIED, "unknown validity level");
    return level;
  }

  bool isValid(enum BlockStateStatus upTo = BLOCK_VALID_TREE) const {
    if ((status & BLOCK_FAILED_MASK) != 0u) {
      // block failed
      return false;
    }
    return isValidUpTo(upTo);
  }

  bool isValidUpTo(enum BlockStateStatus upTo) const {
    auto validityLevel = getValidityLevel();
    VBK_ASSERT_MSG(
        validityLevel != BLOCK_CAN_BE_APPLIED || !hasFlags(BLOCK_FAILED_POP),
        "block %s is both BLOCK_CAN_BE_APPLIED and BLOCK_FAILED_POP which are "
        "mutually exclusive",
        toPrettyString());

    return getValidityLevel() >= upTo;
  }

  void setNull() {
    addon_t::setNull();
    this->pprev = nullptr;
    this->pnext.clear();
    this->height = 0;
    this->status = BLOCK_VALID_UNKNOWN;
    // make it dirty by default
    setDirty();
  }

  void setNullInmemFields() {
    addon_t::setNullInmemFields();
    this->pprev = nullptr;
    this->pnext.clear();
  }

  bool raiseValidity(enum BlockStateStatus upTo) {
    // we can't raise the validity of a block that's known to be invalid due to
    // PoP it's ok to raise the validity of a block invalidated by altchain
    if ((status & BLOCK_FAILED_POP) != 0u) {
      return false;
    }
    if ((status & BLOCK_VALID_MASK) < upTo) {
      VBK_ASSERT_MSG((pprev == nullptr || pprev->getValidityLevel() >= upTo),
                     "attempted to raise the validity level of block %s beyond "
                     "the validity level of its ancestor %s",
                     toPrettyString(),
                     pprev->toPrettyString());

      status = (status & ~BLOCK_VALID_MASK) | upTo;
      return true;
    }
    return false;
  }

  bool lowerValidity(enum BlockStateStatus upTo) {
    // we can't lower the validity of a block that's known to be invalid due to
    // PoP, as that would incorrectly label another validity level as failed.
    // BLOCK_FAILED_POP has to be cleared first via revalidateSubtree it's ok to
    // lower the validity of a block invalidated by altchain
    if ((status & BLOCK_FAILED_POP) != 0u) {
      return false;
    }
    if ((status & BLOCK_VALID_MASK) > upTo) {
      status = (status & ~BLOCK_VALID_MASK) | upTo;
      return true;
    }
    return false;
  }

  void setDirty() { this->dirty = true; }
  void unsetDirty() { this->dirty = false; }
  bool isDirty() const { return this->dirty; }

  void setFlag(enum BlockValidityStatus s) {
    this->status |= s;
    setDirty();
  }
  void unsetFlag(enum BlockValidityStatus s) {
    this->status &= ~s;
    setDirty();
  }

  bool hasFlags(enum BlockValidityStatus s) const { return this->status & s; }

  const hash_t& getHash() const { return header->getHash(); }
  uint32_t getTimestamp() const { return header->getTimestamp(); }
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

  bool canBeATip() const { return isValid(addon_t::validTipLevel); }
  /**
   * The block is a valid tip if it can be a tip and either there are no
   * descendant blocks or none of the descendants can be a tip
   */
  bool isValidTip() const {
    return canBeATip() &&
           (pnext.empty() ||
            std::all_of(pnext.begin(), pnext.end(), [](BlockIndex* index) {
              return !index->canBeATip();
            }));
  }

  /**
   *  Check if all immediate descendants of the block are unapplied
   */
  bool allDescendantsUnapplied() const {
    return pnext.empty() ||
           std::all_of(pnext.begin(), pnext.end(), [](BlockIndex* index) {
             return !index->hasFlags(BLOCK_ACTIVE);
           });
  }

  /**
   *  Check if all immediate descendants of the block are not connected
   */
  bool allDescendantsUnconnected() const {
    return pnext.empty() ||
           std::all_of(pnext.begin(), pnext.end(), [](BlockIndex* index) {
             return !index->isConnected();
           });
  }

  const BlockIndex* getAncestorBlocksBehind(height_t steps) const {
    if (steps < 0 || steps > this->height) {
      return nullptr;
    }

    return this->getAncestor(this->height - steps);
  }

  const BlockIndex* getPrev() const {
    VBK_ASSERT_MSG(pprev == nullptr || getHeight() == pprev->getHeight() + 1,
                   "state corruption: unexpected height of the previous block "
                   "%s of block %s",
                   pprev->toPrettyString(),
                   toPrettyString());

    return pprev;
  }

  BlockIndex* getPrev() {
    const auto& t = as_const(*this);
    return const_cast<BlockIndex*>(t.getPrev());
  }

  bool isDescendantOf(const BlockIndex& ancestor) const {
    return getAncestor(ancestor.getHeight()) == &ancestor;
  }

  bool isAncestorOf(const BlockIndex& descendant) const {
    return descendant.getAncestor(getHeight()) == this;
  }

  //! @returns a pointer to ancestor on height `_height`, if any. If no ancestor
  //! on this height, or height is greater than this block height, returns
  //! nullptr.
  const BlockIndex* getAncestor(height_t _height) const {
    VBK_ASSERT(_height >= 0);
    if (_height > this->height) {
      return nullptr;
    }

    // TODO: this algorithm is not optimal. for O(n) seek backwards until we hit
    // valid height. also it assumes whole blockchain is in memory (pprev is
    // valid until given height)
    const BlockIndex* index = this;
    while (index != nullptr && index->height > _height) {
      index = index->getPrev();
    }

    VBK_ASSERT(index == nullptr || index->height == _height);
    return index;
  }

  //! @overload
  BlockIndex* getAncestor(height_t _height) {
    const auto& t = as_const(*this);
    return const_cast<BlockIndex*>(t.getAncestor(_height));
  }

  std::string toPrettyString(size_t level = 0) const {
    return fmt::sprintf(
        "%s%sBlockIndex(height=%d, hash=%s, next=%d, status=%d, header=%s, %s)",
        std::string(level, ' '),
        Block::name(),
        height,
        HexStr(getHash()),
        pnext.size(),
        status,
        header->toPrettyString(),
        addon_t::toPrettyString());
  }

  std::string toShortPrettyString() const {
    return fmt::sprintf("%s:%d:%s", Block::name(), height, HexStr(getHash()));
  }

  void toVbkEncoding(WriteStream& stream) const {
    using height_t = typename Block::height_t;
    stream.writeBE<height_t>(height);
    header->toRaw(stream);
    stream.writeBE<uint32_t>(status);

    const addon_t* t = this;
    t->toVbkEncoding(stream);
  }

  std::vector<uint8_t> toVbkEncoding() const {
    WriteStream stream;
    toVbkEncoding(stream);
    return stream.data();
  }

  friend bool operator==(const BlockIndex& a, const BlockIndex& b) {
    return a.getStatus() == b.getStatus() && a.getHeight() == b.getHeight() &&
           a.getHeader() == b.getHeader();
  }

  friend bool operator!=(const BlockIndex& a, const BlockIndex& b) {
    return a.getStatus() != b.getStatus() && a.getHeight() != b.getHeight() &&
           a.getHeader() != b.getHeader();
  }

 protected:
  //! height of the entry in the chain
  height_t height = 0;

  //! block header
  std::shared_ptr<block_t> header = std::make_shared<block_t>();

  //! contains status flags
  uint32_t status = BLOCK_VALID_UNKNOWN | BLOCK_DELETED;

  //! (memory only) if true, this block should be written on disk
  bool dirty = false;

  template <typename T>
  friend bool DeserializeFromVbkEncoding(ReadStream& stream,
                                         BlockIndex<T>& out,
                                         ValidationState& state,
                                         typename T::hash_t precalculatedHash);

 private:
  // make it non-copyable
  BlockIndex(const BlockIndex& other) = default;
  BlockIndex& operator=(const BlockIndex& other) = default;
};

/**
 * getForkBlock assumes that:
 *      the block tree is not malformed
 *      the fork block(worst case: genesis/bootstrap block) is in memory
 * the complexity is O(n)
 */
template <typename Block>
const BlockIndex<Block>* getForkBlock(const BlockIndex<Block>& a,
                                      const BlockIndex<Block>& b) {
  const auto initialHeight = std::min(a.getHeight(), b.getHeight());

  for (auto cursorA = a.getAncestor(initialHeight),
            cursorB = b.getAncestor(initialHeight);
       cursorA != nullptr && cursorB != nullptr;
       cursorA = cursorA->getPrev(), cursorB = cursorB->getPrev()) {
    if (cursorA == cursorB) {
      return cursorA;
    }
  }

  // chain `b` is not connected to `a`
  return nullptr;
}

//! a `candidate` is considered outdated if it is behind `finalBlock`, or on
//! same height and not equal to `finalBlock`, or its fork block is outdated
template <typename Block>
bool isBlockOutdated(const BlockIndex<Block>& finalBlock,
                     const BlockIndex<Block>& candidate) {
  // finalBlock is ancestor of candidate
  if (candidate.getAncestor(finalBlock.getHeight()) == &finalBlock) {
    return false;
  }

  // all candidates behind final block are outdated
  if (candidate.getHeight() < finalBlock.getHeight()) {
    return true;
  }

  // all parallel blocks (on same height as final, but not final) are outdated
  if (candidate.getHeight() == finalBlock.getHeight() &&
      &finalBlock != &candidate) {
    return true;
  }

  // candidate is ancestor of finalBlock and within window
  if (finalBlock.getAncestor(candidate.getHeight()) == &candidate) {
    return false;
  }

  // candidate is on a fork
  auto* fork = getForkBlock(finalBlock, candidate);

  // candidate does not connect to finalBlock
  if (fork == nullptr) {
    return true;
  }

  // if fork block is outdated, then candidate is also outdated
  return isBlockOutdated(finalBlock, *fork);
}

//! @private
template <typename Block>
void PrintTo(const BlockIndex<Block>& b, ::std::ostream* os) {
  *os << b.toPrettyString();
}

//! @overload
template <typename Block>
bool DeserializeFromVbkEncoding(
    ReadStream& stream,
    BlockIndex<Block>& out,
    ValidationState& state,
    typename Block::hash_t precalculatedHash = typename Block::hash_t()) {
  const auto& name = Block::name();
  using height_t = typename Block::height_t;
  if (!stream.readBE<height_t>(out.height, state)) {
    return state.Invalid(name + "-block-index-height");
  }
  Block block{};
  if (!DeserializeFromRaw(stream, block, state, precalculatedHash)) {
    return state.Invalid(name + "-block-index-header");
  }
  out.setHeader(block);
  if (!stream.readBE<uint32_t>(out.status, state)) {
    return state.Invalid(name + "-block-index-status");
  }

  using addon_t = typename Block::addon_t;
  addon_t& addon = out;
  if (!DeserializeFromVbkEncoding(stream, addon, state)) {
    return state.Invalid(name + "-block-index-addon");
  }
  out.unsetDirty();
  return true;
}

}  // namespace altintegration
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
