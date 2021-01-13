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
              BlockProvider<typename BlockTreeT::block_t>& provider,
              ValidationState& state) {
  using index_t = typename BlockTreeT::index_t;
  using hash_t = typename index_t::hash_t;

  std::vector<index_t> blocks;

  auto it = provider.getBlockIterator();
  for (it->seek_start(); it->valid(); it->next()) {
    blocks.push_back(it->value());
  }

  hash_t tip_hash;
  if (!provider.getTipHash(tip_hash)) {
    return state.Invalid("bad-tip",
                         fmt::format("Can not read tip of {} block tip",
                                     index_t::block_t::name()));
  }

  if (!LoadTree(out, blocks, tip_hash, state)) {
    return state.Invalid("bad-tree");
  }

  auto* t = out.getBestChain().tip();
  VBK_ASSERT(t != nullptr);

  return true;
}

bool LoadAllTrees(AltBlockTree& tree,
                  BlockProvider<BtcBlock>& btc_provider,
                  BlockProvider<VbkBlock>& vbk_provider,
                  BlockProvider<AltBlock>& alt_provider,
                  ValidationState& state) {
  if (!LoadTree(tree.btc(), btc_provider, state)) {
    return state.Invalid("failed to load btc tree");
  }
  if (!LoadTree(tree.vbk(), vbk_provider, state)) {
    return state.Invalid("failed to load vbk tree");
  }
  if (!LoadTree(tree, alt_provider, state)) {
    return state.Invalid("failed to load alt tree");
  }
  return true;
}

void SaveAllTrees(AltBlockTree& tree, BlockBatchAdaptor& batch) {
  SaveTree(tree.btc(), batch);
  SaveTree(tree.vbk(), batch);
  SaveTree(tree, batch);
}

}  // namespace altintegration