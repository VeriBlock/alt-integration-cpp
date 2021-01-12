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
              BlockHashIterator<typename BlockTreeT::block_t>& it,
              BlockProvider& provider,
              ValidationState& state) {
  using index_t = typename BlockTreeT::index_t;

  if (!it.seek_start()) {
    return state.Invalid("bad-iter", "cannot seek iterator to start");
  }

  std::vector<index_t> blocks;

  while (it.valid()) {
    auto hash = it.value();
    index_t blk;
    if (!provider.getBlock(hash, blk)) {
      return state.Invalid("bad-provider",
                           "cannot get block by the providing hash");
    }

    blocks.push_back(blk);

    if (!it.next()) {
      return state.Invalid("bad-iter", "cannot get next value");
    }
  }

  index_t tip;
  if (!provider.getTip(tip)) {
    return state.Invalid("bad-provider", "cannot get tip");
  }

  if (!LoadTree(out, blocks, tip.getHash(), state)) {
    return state.Invalid("bad-tree");
  }

  auto* t = out.getBestChain().tip();
  VBK_ASSERT(t != nullptr);

  return true;
}

bool LoadAllTrees(AltBlockTree& tree,
                  BlockHashIterator<BtcBlock>& btc_it,
                  BlockHashIterator<VbkBlock>& vbk_it,
                  BlockHashIterator<AltBlock>& alt_it,
                  BlockProvider& provider,
                  ValidationState& state) {
  if (!LoadTree(tree.btc(), btc_it, provider, state)) {
    return state.Invalid("failed to load btc tree");
  }
  if (!LoadTree(tree.vbk(), vbk_it, provider, state)) {
    return state.Invalid("failed to load vbk tree");
  }
  if (!LoadTree(tree, alt_it, provider, state)) {
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