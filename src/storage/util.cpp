// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <algorithm>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/storage/util.hpp>

namespace altintegration {

template <typename BlockTreeT>
bool LoadTree(BlockTreeT& out,
              details::GenericBlockReader<typename BlockTreeT::block_t>& reader,
              ValidationState& state) {
  using index_t = typename BlockTreeT::index_t;
  using hash_t = typename index_t::hash_t;

  std::vector<index_t> blocks;

  auto it = reader.getBlockIterator();
  for (it->seek_start(); it->valid(); it->next()) {
    index_t val;
    if (!it->value(val)) {
      return state.Invalid("bad-value", "Can not get block value");
    }
    blocks.push_back(val);
  }

  hash_t tip_hash;
  if (!reader.getTipHash(tip_hash)) {
    return state.Invalid(index_t::block_t::name() + "-bad-value",
                         "Can not read block tip");
  }

  if (!LoadTree(out, blocks, tip_hash, state)) {
    return state.Invalid("bad-tree");
  }

  auto* t = out.getBestChain().tip();
  VBK_ASSERT(t != nullptr);

  return true;
}

bool LoadAllTrees(PopContext& context, ValidationState& state) {
  if (!LoadTree(context.altTree->btc(),
                *context.blockProvider->getBtcBlockReader(),
                state)) {
    return state.Invalid("failed-to-load-btc-tree");
  }
  if (!LoadTree(context.altTree->vbk(),
                *context.blockProvider->getVbkBlockReader(),
                state)) {
    return state.Invalid("failed-to-load-vbk-tree");
  }
  if (!LoadTree(*context.altTree,
                *context.blockProvider->getAltBlockReader(),
                state)) {
    return state.Invalid("failed-to-load-alt-tree");
  }
  return true;
}

bool SaveAllTrees(PopContext& context, ValidationState& state) {
  if (!SaveTree(context.altTree->btc(),
                *context.blockProvider->getBtcBlockWriter(),
                state)) {
    return state.Invalid("btc-tree-save-failed");
  }
  if (!SaveTree(context.altTree->vbk(),
                *context.blockProvider->getVbkBlockWriter(),
                state)) {
    return state.Invalid("vbk-tree-save-failed");
  }
  if (!SaveTree(*context.altTree,
                *context.blockProvider->getAltBlockWriter(),
                state)) {
    return state.Invalid("alt-tree-save-failed");
  }
  return true;
}

}  // namespace altintegration