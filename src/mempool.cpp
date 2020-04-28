/**
 * Copyright (c) 2019-2020 Xenios SEZC
 * https://www.veriblock.org
 */

#include "veriblock/mempool.hpp"

#include <set>

#include "veriblock/stateless_validation.hpp"

namespace altintegration {

namespace {

bool checkConnectivityWithBlock(const VbkBlock& check_block,
                                const VbkBlock& current_block) {
  static constexpr size_t vbk_prev_block_hash_size =
      decltype(VbkBlock::previousBlock)::size();

  return check_block.previousBlock ==
         current_block.getHash().trimLE<vbk_prev_block_hash_size>();
}

bool checkConnectivityWithTree(const VbkBlock& check_block,
                               const AltTree& tree) {
  return tree.vbk().getBlockIndex(check_block.previousBlock) != nullptr;
}

}  // namespace

void MemPool::uploadVbkContext(const VTB& vtb) {
  static constexpr size_t vbk_prev_block_hash_size =
      decltype(VbkBlock::previousBlock)::size();

  block_index_[vtb.containingBlock.getHash()
                   .trimLE<vbk_prev_block_hash_size>()] = vtb.containingBlock;

  for (const auto& b : vtb.context) {
    block_index_[b.getHash().trimLE<vbk_prev_block_hash_size>()] = b;
  }
}

bool MemPool::fillContext(VbkBlock first_block,
                          std::vector<VbkBlock>& context,
                          AltTree& tree) {
  static constexpr size_t vbk_prev_block_hash_size =
      decltype(VbkBlock::previousBlock)::size();

  while (!checkConnectivityWithTree(first_block, tree)) {
    auto el = block_index_.find(first_block.previousBlock);
    if (el != block_index_.end()) {
      first_block = el->second;
      context.push_back(first_block);
    } else {
      return false;
    }
  }

  // clear from the block_index_
  for (const auto& b : context) {
    block_index_.erase(b.getHash().trimLE<vbk_prev_block_hash_size>());
  }

  // reverse context
  std::reverse(context.begin(), context.end());

  return true;
}

void MemPool::fillVTBs(std::vector<VTB>& vtbs,
                       const std::vector<VbkBlock>& vbk_contex) {
  for (const auto& b : vbk_contex) {
    for (auto vtb_it = stored_vtbs_.begin(); vtb_it != stored_vtbs_.end();) {
      if (vtb_it->containingBlock == b) {
        vtbs.push_back(*vtb_it);
        vtb_it = stored_vtbs_.erase(vtb_it);
        continue;
      }

      ++vtb_it;
    }
  }
}

void MemPool::uploadVbkContext(const ATV& atv) {
  static constexpr size_t vbk_prev_block_hash_size =
      decltype(VbkBlock::previousBlock)::size();

  block_index_[atv.containingBlock.getHash()
                   .trimLE<vbk_prev_block_hash_size>()] = atv.containingBlock;

  for (const auto& b : atv.context) {
    block_index_[b.getHash().trimLE<vbk_prev_block_hash_size>()] = b;
  }
}

bool MemPool::submitATV(const std::vector<ATV>& atvs, ValidationState& state) {
  for (size_t i = 0; i < atvs.size(); ++i) {
    if (!checkATV(atvs[i], state, *alt_chain_params_, *vbk_chain_params_)) {
      return state.Invalid("mempool-submit-atv", i);
    }

    uploadVbkContext(atvs[i]);
    stored_atvs_.push_back(atvs[i]);
    stored_atvs_.rbegin()->context.clear();
  }

  return true;
}

bool MemPool::submitVTB(const std::vector<VTB>& vtbs, ValidationState& state) {
  for (size_t i = 0; i < vtbs.size(); ++i) {
    if (!checkVTB(vtbs[i], state, *vbk_chain_params_, *btc_chain_params_)) {
      state.Invalid("mempool-submit-vtb", i);
    }

    uploadVbkContext(vtbs[i]);
    stored_vtbs_.push_back(vtbs[i]);
    stored_vtbs_.rbegin()->context.clear();
  }

  return true;
}

std::vector<AltPopTx> MemPool::getPop(const AltBlock& current_block,
                                      AltTree& tree,
                                      ValidationState& state) {
  auto hash = current_block.getHash();
  bool ret = tree.setState(hash, state);
  (void)ret;
  assert(ret);

  AltBlock tempBlock = {{0, 0, 0, 0, 0, 0, 0, 1},
                        hash,
                        current_block.timestamp,
                        current_block.height + 1};

  auto applyPayloads = [&](const AltPopTx& popTx) -> bool {
    AltPayloads payloads;
    payloads.altPopTx = popTx;
    payloads.containingBlock = tempBlock;

    // find endorsed block
    auto endorsed_hash = hasher(popTx.atv.transaction.publicationData.header);
    auto endorsed_block_index = tree.getBlockIndex(endorsed_hash);
    if (!endorsed_block_index) {
      return false;
    }
    payloads.endorsed = *endorsed_block_index->header;

    bool ret = tree.acceptBlock(tempBlock, state);
    assert(ret);

    return tree.addPayloads(tempBlock, {payloads}, state);
  };

  std::vector<AltPopTx> popTxs;
  for (auto atv_it = stored_atvs_.begin(); atv_it != stored_atvs_.end();) {
    auto& atv = *atv_it;
    AltPopTx popTx;
    VbkBlock first_block =
        !atv.context.empty() ? atv.context[0] : atv.containingBlock;

    if (!checkConnectivityWithTree(first_block, tree)) {
      if (fillContext(first_block, popTx.vbk_context, tree)) {
        fillVTBs(popTx.vtbs, popTx.vbk_context);
        popTx.atv = atv;
        if (applyPayloads(popTx)) {
          popTxs.push_back(popTx);
        }
        atv_it = stored_atvs_.erase(atv_it);
        continue;
      }
    } else {
      popTx.atv = atv;
      if (applyPayloads(popTx)) {
        popTxs.push_back(popTx);
      }
      atv_it = stored_atvs_.erase(atv_it);
      continue;
    }

    ++atv_it;
  }

  // clear tree from temp endorsement
  tree.invalidateBlockByHash(tempBlock.getHash());

  return popTxs;
}

}  // namespace altintegration
