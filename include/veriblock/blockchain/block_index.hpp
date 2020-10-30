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
#include <veriblock/blockchain/block_status.hpp>
#include <veriblock/blockchain/command.hpp>
#include <veriblock/blockchain/command_group.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/validation_state.hpp>
#include <veriblock/write_stream.hpp>

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

  bool isValid(enum BlockStatus upTo = BLOCK_VALID_TREE) const {
    if ((status & BLOCK_FAILED_MASK) != 0u) {
      // block failed
      return false;
    }
    return isValidUpTo(upTo);
  }

  bool isValidUpTo(enum BlockStatus upTo) const {
    VBK_ASSERT_MSG(!(upTo & ~BLOCK_VALID_MASK),
                   "Only validity flags are allowed");

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

  bool raiseValidity(enum BlockStatus upTo) {
    VBK_ASSERT(!(upTo & ~BLOCK_VALID_MASK));  // Only validity flags allowed.
    // we can't raise the validity of a block that's known to be invalid due to
    // PoP it's ok to raise the validity of a block invalidated by altchain
    if ((status & BLOCK_FAILED_POP) != 0u) {
      return false;
    }
    if ((status & BLOCK_VALID_MASK) < upTo) {
      // FIXME: this should be enabled once BLOCK_HAS_PAYLOADS is gone from the
      // list of stateful validity levels
      // VBK_ASSERT_MSG(pprev == nullptr || pprev->getValidityLevel() >= upTo,
      // "attempted to raise the validity level of block %s beyond the validity
      // level of its ancestor %s", toPrettyString(), pprev->toPrettyString());
      status = (status & ~BLOCK_VALID_MASK) | upTo;
      return true;
    }
    return false;
  }

  bool lowerValidity(enum BlockStatus upTo) {
    VBK_ASSERT(!(upTo & ~BLOCK_VALID_MASK));  // Only validity flags allowed.
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

  void setFlag(enum BlockStatus s) {
    this->status |= s;
    setDirty();
  }
  void unsetFlag(enum BlockStatus s) {
    this->status &= ~s;
    setDirty();
  }

  bool hasFlags(BlockStatus s) const { return this->status & s; }

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
             return !index->hasFlags(BLOCK_APPLIED);
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

  BlockIndex* getPrev() const {
    VBK_ASSERT_MSG(pprev == nullptr || getHeight() == pprev->getHeight() + 1,
                   "state corruption: unexpected height of the previous block "
                   "%s of block %s",
                   pprev->toPrettyString(),
                   toPrettyString());

    return pprev;
  }

  bool isDescendantOf(const BlockIndex& ancestor) const {
    return getAncestor(ancestor.getHeight()) == &ancestor;
  }

  bool isAncestorOf(const BlockIndex& descendant) const {
    return descendant.getAncestor(getHeight()) == this;
  }

  BlockIndex* getAncestor(height_t _height) const {
    VBK_ASSERT(_height >= 0);
    if (_height > this->height) {
      return nullptr;
    }

    // TODO: this algorithm is not optimal. for O(n) seek backwards until we hit
    // valid height. also it assumes whole blockchain is in memory (pprev is
    // valid until given height)
    BlockIndex* index = const_cast<BlockIndex*>(this);
    while (index != nullptr && index->height > _height) {
      index = index->getPrev();
    }

    VBK_ASSERT(index == nullptr || index->height == _height);
    return index;
  }

  std::string toPrettyString(size_t level = 0) const {
    return fmt::sprintf("%s%sBlockIndex(height=%d, hash=%s, status=%d, %s)",
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
    stream.writeBE<uint32_t>(status);
    addon_t::toRaw(stream);
  }

  void initFromRaw(ReadStream& stream) {
    height = stream.readBE<uint32_t>();
    header = std::make_shared<Block>(Block::fromRaw(stream));
    status = stream.readBE<uint32_t>();
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

  template <bool EnableBool = true,
            typename = typename std::enable_if<
                (std::is_same<Block, VbkBlock>::value && EnableBool)>::type>
  void toRawAddHash(WriteStream& stream) const {
    stream.writeBE<uint32_t>(height);
    header->toRawAddHash(stream);
    stream.writeBE<uint32_t>(status);
    addon_t::toRaw(stream);
  }

  template <typename = typename std::enable_if<
                (std::is_same<Block, VbkBlock>::value)>::type>
  std::vector<uint8_t> toRawAddHash() const {
    WriteStream stream;
    toRawAddHash(stream);
    return stream.data();
  }

  template <bool EnableBool = true,
            typename = typename std::enable_if<
                (std::is_same<Block, VbkBlock>::value && EnableBool)>::type>
  void initFromRawAddHash(ReadStream& stream) {
    height = stream.readBE<uint32_t>();
    header = std::make_shared<VbkBlock>(VbkBlock::fromRawAddHash(stream));
    status = stream.readBE<uint32_t>();
    addon_t::initAddonFromRaw(stream);
    setDirty();
  }

  template <bool EnableBool = true,
            typename = typename std::enable_if<
                (std::is_same<Block, VbkBlock>::value && EnableBool)>::type>
  static BlockIndex fromRawAddHash(ReadStream& stream) {
    BlockIndex index{};
    index.initFromRawAddHash(stream);
    return index;
  }

  template <bool EnableBool = true,
            typename = typename std::enable_if<
                (std::is_same<Block, VbkBlock>::value && EnableBool)>::type>
  static BlockIndex fromRawAddHash(Slice<const uint8_t> bytes) {
    ReadStream stream(bytes);
    return fromRawAddHash(stream);
  }

 protected:
  //! height of the entry in the chain
  height_t height = 0;

  //! block header
  std::shared_ptr<block_t> header{};

  //! contains status flags
  uint32_t status = BLOCK_VALID_UNKNOWN;

  //! (memory only) if true, this block should be written on disk
  bool dirty = false;
};

/**
 * getForkBlock assumes that:
 *      the block tree is not malformed
 *      the fork block(worst case: genesis/bootstrap block) is in memory
 * the complexity is O(n)
 */
template <typename Block>
BlockIndex<Block>& getForkBlock(BlockIndex<Block>& a, BlockIndex<Block>& b) {
  const auto initialHeight = std::min(a.getHeight(), b.getHeight());

  for (auto cursorA = a.getAncestor(initialHeight),
            cursorB = b.getAncestor(initialHeight);
       cursorA != nullptr && cursorB != nullptr;
       cursorA = cursorA->getPrev(), cursorB = cursorB->getPrev()) {
    if (cursorA == cursorB) {
      return *cursorA;
    }
  }

  VBK_ASSERT_MSG(false,
                 "blocks %s and %s must be part of the same tree",
                 a.toPrettyString(),
                 b.toPrettyString());
}

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
