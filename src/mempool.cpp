// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/mempool.hpp"

#include <deque>

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
                               const VbkBlockTree& tree) {
  return tree.getBlockIndex(check_block.previousBlock) != nullptr;
}

}  // namespace

void MemPool::uploadVbkContext(const VTB& vtb) {
  block_index_[vtb.containingBlock.getHash()
                   .trimLE<vbk_prev_block_hash_size>()] = vtb.containingBlock;

  for (const auto& b : vtb.context) {
    block_index_[b.getHash().trimLE<vbk_prev_block_hash_size>()] = b;
  }
}

void MemPool::uploadVbkContext(const ATV& atv) {
  block_index_[atv.containingBlock.getHash()
                   .trimLE<vbk_prev_block_hash_size>()] = atv.containingBlock;

  for (const auto& b : atv.context) {
    block_index_[b.getHash().trimLE<vbk_prev_block_hash_size>()] = b;
  }
}

bool MemPool::fillContext(VbkBlock first_block,
                          std::vector<VbkBlock>& context,
                          AltTree& tree) {
  while (!checkConnectivityWithTree(first_block, tree.vbk())) {
    auto el = block_index_.find(first_block.previousBlock);
    if (el != block_index_.end()) {
      first_block = el->second;
      context.push_back(first_block);
    } else {
      return false;
    }
  }

  // reverse context
  std::reverse(context.begin(), context.end());

  return true;
}

void MemPool::fillVTBs(std::vector<VTB>& vtbs,
                       const std::vector<VbkBlock>& vbk_contex) {
  for (const auto& b : vbk_contex) {
    for (auto vtb_it = stored_vtbs_.begin(); vtb_it != stored_vtbs_.end();
         ++vtb_it) {
      if (vtb_it->second.containingBlock == b) {
        vtbs.push_back(vtb_it->second);
      }
    }
  }
}

bool MemPool::applyPayloads(const AltBlock& hack_block,
                            AltPopTx& altPopTx,
                            AltTree& tree,
                            ValidationState& state) {
  AltPayloads payloads;
  payloads.altPopTx = altPopTx;
  payloads.containingBlock = hack_block;

  // find endorsed block
  auto endorsed_hash = hasher(altPopTx.atv.transaction.publicationData.header);
  auto endorsed_block_index = tree.getBlockIndex(endorsed_hash);
  if (!endorsed_block_index) {
    return false;
  }
  payloads.endorsed = *endorsed_block_index->header;

  bool ret = tree.acceptBlock(hack_block, state);
  assert(ret);

  // apply vbk_context
  for (const auto& b : altPopTx.vbk_context) {
    ret = tree.vbk().acceptBlock(b, state);
    assert(ret);
  }

  auto genesis_height = tree.vbk().getParams().getGenesisBlock().height;
  auto settlement_interval =
      tree.vbk().getParams().getEndorsementSettlementInterval();
  // check VTB endorsements
  for (auto it = altPopTx.vtbs.begin(); it != altPopTx.vtbs.end();) {
    auto& vtb = *it;
    auto* containing_block_index =
        tree.vbk().getBlockIndex(vtb.containingBlock.getHash());

    auto start_height =
        genesis_height > (containing_block_index->height - settlement_interval)
            ? genesis_height
            : (containing_block_index->height - settlement_interval);

    auto endorsement = BtcEndorsement::fromContainer(vtb);
    Chain<BlockIndex<VbkBlock>> chain(start_height, containing_block_index);
    auto duplicate =
        chain.findBlockContainingEndorsement(endorsement, settlement_interval);

    // invalid vtb
    if (duplicate) {
      // remove from storage
      stored_vtbs_.erase(vtb.getId());

      it = altPopTx.vtbs.erase(it);
      continue;
    }
    ++it;
  }

  return tree.addPayloads(hack_block, {payloads}, state);
}

bool MemPool::submitATV(const std::vector<ATV>& atvs, ValidationState& state) {
  for (size_t i = 0; i < atvs.size(); ++i) {
    if (!checkATV(atvs[i], state, *alt_chain_params_, *vbk_chain_params_)) {
      return state.Invalid("mempool-submit-atv", i);
    }

    uploadVbkContext(atvs[i]);
    auto pair = std::make_pair(atvs[i].getId(), atvs[i]);
    // clear context
    pair.second.context.clear();

    stored_atvs_.insert(pair);
  }

  return true;
}

bool MemPool::submitVTB(const std::vector<VTB>& vtbs, ValidationState& state) {
  for (size_t i = 0; i < vtbs.size(); ++i) {
    if (!checkVTB(vtbs[i], state, *vbk_chain_params_, *btc_chain_params_)) {
      return state.Invalid("mempool-submit-vtb", i);
    }

    uploadVbkContext(vtbs[i]);
    auto pair = std::make_pair(vtbs[i].getId(), vtbs[i]);
    // clear contex
    pair.second.context.clear();

    stored_vtbs_.insert(pair);
  }

  return true;
}

std::vector<AltPopTx> MemPool::getPop(const AltBlock& current_block,
                                      AltTree& tree,
                                      ValidationState& state) {
  bool ret = tree.setState(current_block.getHash(), state);
  (void)ret;
  assert(ret);

  AltBlock hack_block;
  hack_block.hash = {0, 0, 0, 0, 0, 0, 0, 1};
  hack_block.previousBlock = current_block.getHash();
  hack_block.timestamp = current_block.timestamp + 1;
  hack_block.height = current_block.height + 1;

  std::deque<ATV> atvs;
  for (auto it = stored_atvs_.begin(); it != stored_atvs_.end(); ++it) {
    atvs.push_back(it->second);
  }

  auto atv_comparator = [](const ATV& el1, const ATV& el2) -> bool {
    return el1.containingBlock.height > el2.containingBlock.height;
  };
  std::make_heap(atvs.begin(), atvs.end(), atv_comparator);

  std::vector<AltPopTx> popTxs;
  while (!atvs.empty()) {
    auto& atv = atvs.front();
    AltPopTx popTx;
    VbkBlock first_block =
        !atv.context.empty() ? atv.context[0] : atv.containingBlock;

    if (!checkConnectivityWithTree(first_block, tree.vbk())) {
      if (fillContext(first_block, popTx.vbk_context, tree)) {
        fillVTBs(popTx.vtbs, popTx.vbk_context);
        popTx.atv = atv;
        popTx.hasAtv = true;
        if (applyPayloads(hack_block, popTx, tree, state)) {
          popTxs.push_back(popTx);
        }
      }
    } else {
      popTx.atv = atv;
      popTx.hasAtv = true;
      if (applyPayloads(hack_block, popTx, tree, state)) {
        popTxs.push_back(popTx);
      }
    }

    atvs.pop_front();
    std::make_heap(atvs.begin(), atvs.end(), atv_comparator);
  }

  // clear tree from temp endorsement
  tree.invalidateBlockByHash(hack_block.getHash());

  return popTxs;
}

void MemPool::removePayloads(const std::vector<AltPopTx>& altPopTxs) {
  for (const auto& tx : altPopTxs) {
    // clear context
    for (const auto& b : tx.vbk_context) {
      block_index_.erase(b.getHash().trimLE<vbk_prev_block_hash_size>());
    }

    // clear atv
    stored_atvs_.erase(tx.atv.getId());

    // clear vtbs
    for (const auto& vtb : tx.vtbs) {
      stored_vtbs_.erase(vtb.getId());
    }
  }
}

}  // namespace altintegration
