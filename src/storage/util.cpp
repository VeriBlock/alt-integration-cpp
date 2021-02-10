// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <algorithm>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/crypto/progpow.hpp>
#include <veriblock/storage/util.hpp>

namespace altintegration {

namespace detail {

template <typename Block>
using OnBlockCallback_t =
    std::function<void(typename Block::hash_t, const BlockIndex<Block>&)>;

template <typename BlockTreeT>
bool LoadTree(
    BlockTreeT& out,
    BlockReader& storage,
    ValidationState& state,
    const OnBlockCallback_t<typename BlockTreeT::block_t>& onBlock = {}) {
  using index_t = typename BlockTreeT::index_t;
  using block_t = typename BlockTreeT::block_t;
  using hash_t = typename index_t::hash_t;

  std::vector<std::unique_ptr<index_t>> blocks;

  auto it = storage.getBlockIterator<block_t>();
  for (it->seek_start(); it->valid(); it->next()) {
    auto val = make_unique<index_t>(nullptr);
    if (!it->value(*val)) {
      return state.Invalid("bad-value", "Can not read block data");
    }
    // if callback is supplied, execute it
    if (onBlock) {
      hash_t hash;
      if (!it->key(hash)) {
        return state.Invalid("bad-key", "Can not read block key");
      }
      onBlock(hash, *val);
    }
    blocks.push_back(std::move(val));
  }

  hash_t tip_hash;
  bool res = storage.getTip<block_t>(tip_hash);

  if (!res && !blocks.empty()) {
    return state.Invalid(index_t::block_t::name() + "-bad-tip",
                         "Can not read block tip");
  }

  if (res && blocks.empty()) {
    return state.Invalid(index_t::block_t::name() + "-state-corruption",
                         "Can not read blocks");
  }

  // skip loading because storage is empty
  if (!res && blocks.empty()) {
    return true;
  }

  if (!LoadBlocks(out, blocks, tip_hash, state)) {
    return state.Invalid("bad-tree");
  }

  auto* t = out.getBestChain().tip();
  VBK_ASSERT(t != nullptr);

  return true;
}

}  // namespace detail

bool LoadAllTrees(PopContext& context,
                  BlockReader& storage,
                  ValidationState& state) {
  if (!detail::LoadTree(context.altTree->btc(), storage, state)) {
    return state.Invalid("failed-to-load-btc-tree");
  }
  if (!detail::LoadTree(
          context.altTree->vbk(),
          storage,
          state,
          // on every block, take its hash and warmup progpow header cache
          [](VbkBlock::hash_t hash, const BlockIndex<VbkBlock>& index) {
            auto serializedHeader = SerializeToRaw(index.getHeader());
            progpow::insertHeaderCacheEntry(serializedHeader, std::move(hash));
          })) {
    return state.Invalid("failed-to-load-vbk-tree");
  }
  if (!detail::LoadTree(*context.altTree, storage, state)) {
    return state.Invalid("failed-to-load-alt-tree");
  }
  return true;
}

void SaveAllTrees(const AltBlockTree& tree, BlockBatch& batch) {
  SaveTree(tree.btc(), batch);
  SaveTree(tree.vbk(), batch);
  SaveTree(tree, batch);
}

}  // namespace altintegration