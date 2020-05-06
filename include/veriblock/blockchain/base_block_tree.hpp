// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_BASE_BLOCK_TREE_HPP
#define ALTINTEGRATION_BASE_BLOCK_TREE_HPP

#include <unordered_map>
#include <unordered_set>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/tree_algo.hpp>

namespace altintegration {

template <typename Block>
struct BaseBlockTree {
  using block_t = Block;
  using hash_t = typename Block::hash_t;
  using prev_block_hash_t = decltype(Block::previousBlock);
  using index_t = BlockIndex<Block>;
  using block_index_t =
      std::unordered_map<prev_block_hash_t, std::shared_ptr<index_t>>;

  const std::unordered_set<index_t*>& getTips() const { return tips_; }
  const block_index_t& getBlocks() const { return blocks_; }

  virtual ~BaseBlockTree() = default;

  template <typename T,
            typename = typename std::enable_if<
                std::is_same<T, hash_t>::value ||
                std::is_same<T, prev_block_hash_t>::value>::type>
  index_t* getBlockIndex(const T& hash) const {
    auto shortHash = makePrevHash(hash);
    auto it = blocks_.find(shortHash);
    return it == blocks_.end() ? nullptr : it->second.get();
  }

 protected:
  void removeSingleBlock(index_t& block) {
    // if it is a tip, we also remove it
    tips_.erase(&block);
    block.pnext.clear();

    auto shortHash = makePrevHash(block.getHash());
    blocks_.erase(shortHash);
  }

  void removeSubtree(index_t& index,
                     const std::function<void(index_t&)>& onBlock) {
    // save ptr to a previous block
    auto* prev = index.pprev;

    // remove this block from 'pnext' set of previous block
    if (prev) {
      prev->pnext.erase(&index);
    }

    forEachNodePostorder<block_t>(index, [&](index_t& next) {
      onBlock(next);
      removeSingleBlock(next);
    });

    // after removal, try to add tip
    tryAddTip(prev);
  }

  void invalidateBlock(index_t& toBeInvalidated,
                       enum BlockStatus reason,
                       const std::function<bool(index_t&)>& onBlock) {
    toBeInvalidated.setFlag(reason);
    tryAddTip(toBeInvalidated.pprev);

    forEachNextNodePreorder<block_t>(toBeInvalidated,
                                     [&](index_t& index) -> bool {
                                       bool ret = onBlock(index);
                                       index.setFlag(BLOCK_FAILED_CHILD);
                                       return ret;
                                     });

    for (auto it = tips_.begin(); it != tips_.end();) {
      index_t* index = *it;
      if (!index->isValid()) {
        it = tips_.erase(it);
      } else {
        ++it;
      }
    }
  }

  void tryAddTip(index_t* index) {
    assert(index);

    auto it = tips_.find(index->pprev);
    if (it != tips_.end()) {
      // we found prev block in chainTips
      tips_.erase(it);
    }

    // this can be a tip ONLY if pnext is empty OR all items in pnext are
    // invalid
    auto& pn = index->pnext;
    if (pn.empty() || std::all_of(pn.begin(), pn.end(), [](index_t* n) {
          return !n->isValid();
        })) {
      tips_.insert(index);
    }
  }

  index_t* touchBlockIndex(const hash_t& hash) {
    auto shortHash = makePrevHash(hash);
    auto it = blocks_.find(shortHash);
    if (it != blocks_.end()) {
      return it->second.get();
    }

    auto newIndex = std::make_shared<index_t>();
    it = blocks_.insert({shortHash, std::move(newIndex)}).first;
    return it->second.get();
  }

  index_t* doInsertBlockHeader(const std::shared_ptr<block_t>& header) {
    assert(header != nullptr);

    index_t* current = touchBlockIndex(header->getHash());
    current->header = header;
    current->pprev = getBlockIndex(header->previousBlock);

    if (current->pprev != nullptr) {
      // prev block found
      current->height = current->pprev->height + 1;
      current->pprev->pnext.insert(current);
    } else {
      current->height = 0;
    }

    tryAddTip(current);

    return current;
  }

  std::string toPrettyString(size_t level = 0) const {
    std::ostringstream ss;
    std::string pad(level, ' ');
    ss << pad << "{blocks=\n";
    // sort blocks by height
    std::vector<std::pair<int, index_t*>> byheight;
    byheight.reserve(blocks_.size());
    for (const auto& p : blocks_) {
      byheight.push_back({p.second->height, p.second.get()});
    }
    std::sort(byheight.rbegin(), byheight.rend());
    for (const auto& p : byheight) {
      ss << p.second->toPrettyString(level + 2) << "\n";
    }
    ss << pad << "}\n";
    ss << pad << "{tips=\n";
    for(const auto* tip: tips_) {
      ss << tip->toPrettyString(level + 2) << "\n";
    }
    ss << pad << "}";
    return ss.str();
  }

 private:
  // HACK: see comment below.
  template <typename T>
  inline prev_block_hash_t makePrevHash(const T& h) const {
    // given any type T, just return an implicit cast to prev_block_hash_t
    return h;
  }

 protected:
  //! stores ALL blocks, including valid and invalid
  block_index_t blocks_;
  //! stores ONLY VALID tips
  std::unordered_set<index_t*> tips_;
};

// HACK: getBlockIndex accepts either hash_t or prev_block_hash_t
// then, depending on what it received, it should do trim LE on full hash to
// receive short hash, which is stored inside a map. In this weird case, when
// Block=VbkBlock, we may call `getBlockIndex(block->previousBlock)`, it is a
// call `getBlockIndex(Blob<12>). But when `getBlockIndex` accepts it, it does
// an implicit cast to full hash (hash_t), adding zeroes in the end. Then,
// .trimLE returns 12 zeroes.
//
// This hack allows us to inject explicit conversion hash_t (Blob<24>) ->
// prev_block_hash_t (Blob<12>).
template <>
template <>
inline BaseBlockTree<VbkBlock>::prev_block_hash_t
BaseBlockTree<VbkBlock>::makePrevHash<BaseBlockTree<VbkBlock>::hash_t>(
    const hash_t& h) const {
  // do an explicit cast from hash_t -> prev_block_hash_t
  return h.template trimLE<prev_block_hash_t::size()>();
}

}  // namespace altintegration

#endif  // ALTINTEGRATION_BASE_BLOCK_TREE_HPP
