// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <algorithm>
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/crypto/progpow.hpp>
#include <veriblock/pop/pop_context.hpp>
#include <veriblock/pop/storage/util.hpp>

namespace altintegration {

namespace detail {

template <typename Block>
using OnBlockCallback_t =
    std::function<void(typename Block::hash_t, const StoredBlockIndex<Block>&)>;

template <typename Index>
bool loadBlocksAndTip(
    std::vector<Index>& out,
    typename Index::block_t::hash_t& tipout,
    const BlockReader& storage,
    ValidationState& state,
    const OnBlockCallback_t<typename Index::block_t>& onBlock = {}) {
  using block_t = typename Index::block_t;
  using hash_t = typename block_t::hash_t;

  auto it = storage.getBlockIterator<block_t>();
  for (it->seek_start(); it->valid(); it->next()) {
    Index val;
    if (!it->value(val)) {
      return state.Invalid("bad-value", "Can not read block data");
    }
    if (val.isDeleted()) continue;
    // if callback is supplied, execute it
    if (onBlock) {
      hash_t hash;
      if (!it->key(hash)) {
        return state.Invalid("bad-key", "Can not read block key");
      }
      onBlock(hash, val);
    }
    out.push_back(val);
  }

  bool res = storage.getTip<block_t>(tipout);

  if (!res && !out.empty()) {
    return state.Invalid(block_t::name() + "-bad-tip",
                         "Can not read block tip");
  }

  if (res && out.empty()) {
    return state.Invalid(block_t::name() + "-state-corruption",
                         "Can not read blocks");
  }

  return true;
}

}  // namespace detail

bool loadTrees(AltBlockTree& tree, bool fast_load, ValidationState& state) {
  std::vector<typename BtcBlockTree::stored_index_t> btcblocks;
  typename BtcBlock::hash_t btctip;
  if (!detail::loadBlocksAndTip(
          btcblocks, btctip, tree.getBlockProvider(), state)) {
    return state.Invalid("load-btc-tree-blocks");
  }
  std::vector<typename VbkBlockTree::stored_index_t> vbkblocks;
  typename VbkBlock::hash_t vbktip;
  if (!detail::loadBlocksAndTip(
          vbkblocks,
          vbktip,
          tree.getBlockProvider(),
          state,  // on every block, take its hash and warmup progpow header
                  // cache
          [](VbkBlock::hash_t hash, const StoredBlockIndex<VbkBlock>& index) {
            auto serializedHeader = SerializeToRaw(*index.header);
            setPrecalculatedHash(*index.header, hash);
            progpow::insertHeaderCacheEntry(serializedHeader, std::move(hash));
          })) {
    return state.Invalid("load-vbk-tree-blocks");
  }
  std::vector<typename AltBlockTree::stored_index_t> altblocks;
  typename AltBlock::hash_t alttip;
  if (!detail::loadBlocksAndTip(
          altblocks, alttip, tree.getBlockProvider(), state)) {
    return state.Invalid("load-alt-tree-blocks");
  }

  if (!loadTree(tree.btc(), btctip, btcblocks, fast_load, state)) {
    return state.Invalid("failed-to-load-btc-tree");
  }

  if (!loadTree(tree.vbk(), vbktip, vbkblocks, fast_load, state)) {
    return state.Invalid("failed-to-load-vbk-tree");
  }

  if (!loadTree(tree, alttip, altblocks, fast_load, state)) {
    return state.Invalid("failed-to-load-alt-tree");
  }

  return true;
}

template <typename BlockIndexT>
void validateBlockIndex(const BlockIndexT&) {}

template <>
void validateBlockIndex(const BlockIndex<VbkBlock>& index) {
  const auto& vtbids = index.getPayloadIds<VTB>();
  VBK_ASSERT_MSG(!hasDuplicateIdsOf<VTB>(vtbids), "Duplicate VTB IDs");
}

void saveTrees(const AltBlockTree& tree, BlockBatch& batch) {
  saveTree(tree.btc(), batch);
  saveTree(tree.vbk(), batch);
  saveTree(tree, batch);
}

}  // namespace altintegration
