// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP
#define VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP

#include <vector>
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/logger.hpp>
#include <veriblock/pop/storage/block_reader.hpp>
#include <veriblock/pop/validation_state.hpp>

#include "block_batch.hpp"

namespace altintegration {

namespace detail {
template <typename BlockTreeT>
bool validateLoadBlock(const BlockTreeT&,
                       const typename BlockTreeT::stored_index_t&,
                       ValidationState&);

template <>
bool validateLoadBlock(const AltBlockTree& tree,
                       const typename AltBlockTree::stored_index_t& index,
                       ValidationState& state);

template <>
bool validateLoadBlock(const VbkBlockTree& tree,
                       const typename VbkBlockTree::stored_index_t& index,
                       ValidationState& state);

template <>
bool validateLoadBlock(const BtcBlockTree&,
                       const typename BtcBlockTree::stored_index_t&,
                       ValidationState&);

template <typename BlockTreeT>
bool loadValidateTree(
    const BlockTreeT& tree,
    const std::vector<typename BlockTreeT::stored_index_t>& blocks,
    ValidationState& state) {
  VBK_LOG_DEBUG("Entered method");

  using block_t = typename BlockTreeT::block_t;

  for (const auto& block : blocks) {
    if (!validateLoadBlock(tree, block, state)) {
      return state.Invalid("load-validate-tree",
                           format("Invalid stored {} block {}",
                                  block_t::name(),
                                  block.toPrettyString()));
    }
  }
  return true;
}

}  // namespace detail

//! efficiently loads `blocks` into tree (they must be sorted by height) and
//! does validation of these blocks. Sets tip after loading.
//! @invariant NOT atomic
template <typename BlockTreeT>
bool loadTree(BlockTreeT& tree,
              const typename BlockTreeT::hash_t& tiphash,
              std::vector<typename BlockTreeT::stored_index_t>& blocks,
              ValidationState& state) {
  using stored_index_t = typename BlockTreeT::stored_index_t;

  if (blocks.size() == 0) return true;

  VBK_LOG_INFO("Loading %d %s blocks with tip %s",
               blocks.size(),
               BlockTreeT::block_t::name(),
               HexStr(tiphash));
  VBK_ASSERT(tree.isBootstrapped() && "tree must be bootstrapped");

  // first, sort them by height
  std::sort(blocks.begin(),
            blocks.end(),
            [](const stored_index_t& a, const stored_index_t& b) {
              return a.height < b.height;
            });

  for (const auto& block : blocks) {
    // load blocks one by one
    if (!tree.loadBlockForward(block, state)) {
      return state.Invalid("load-tree");
    }
  }

  if (!tree.loadTip(tiphash, state)) {
    return state.Invalid("load-tree");
  }

  auto* t = tree.getBestChain().tip();
  VBK_ASSERT(t != nullptr);

  return true;
}

//! @private
template <typename BlockIndexT>
void validateBlockIndex(const BlockIndexT&);

//! @private
template <typename BlockTreeT>
void saveTree(
    BlockTreeT& tree,
    BlockBatch& batch,
    std::function<void(const typename BlockTreeT::index_t&)> validator) {
  VBK_LOG_DEBUG("Entered method");

  using index_t = typename BlockTreeT::index_t;
  std::vector<const index_t*> dirty_indices;

  // map pair<hash, shared_ptr<index_t>> to vector<index_t*>
  for (auto& index : tree.getAllBlocks()) {
    if (index->isDirty()) {
      index->unsetDirty();
      dirty_indices.push_back(index);
    }
  }

  // sort by height in descending order, because we need to calculate index hash
  // during saving. this is needed to be progpow-cache friendly, as cache will
  // be warm for last blocks.
  std::sort(dirty_indices.begin(),
            dirty_indices.end(),
            [](const index_t* a, const index_t* b) {
              return b->getHeight() < a->getHeight();
            });

  // write indices
  for (const index_t* index : dirty_indices) {
    validator(*index);
    batch.writeBlock(index->getHash(),
                     tree.makePrevHash(index->getHash()),
                     index->toStoredBlockIndex());
  }

  batch.writeTip(tree.getBestChain().tip()->getHash());
}

//! @private
template <typename BlockTreeT>
void saveTree(BlockTreeT& tree, BlockBatch& batch) {
  saveTree(tree, batch, &validateBlockIndex<typename BlockTreeT::index_t>);
}

struct AltBlockTree;

//! Save all (BTC/VBK/ALT) trees on disk in a single Batch.
void saveTrees(const AltBlockTree& tree, BlockBatch& batch);

//! Load all (ALT/VBK/BTC) trees from disk into memory.
bool loadTrees(AltBlockTree& tree, ValidationState& state);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP
