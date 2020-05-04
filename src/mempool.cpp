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
  return check_block.previousBlock == current_block.getShortHash();
}

bool checkConnectivityWithTree(const VbkBlock& check_block,
                               const VbkBlockTree& tree) {
  return tree.getBlockIndex(check_block.previousBlock) != nullptr;
}

}  // namespace

void MemPool::uploadVbkContext(const VTB& vtb) {
  block_index_[vtb.containingBlock.getShortHash()] = vtb.containingBlock;

  for (const auto& b : vtb.context) {
    block_index_[b.getShortHash()] = b;
  }
}

void MemPool::uploadVbkContext(const ATV& atv) {
  block_index_[atv.containingBlock.getShortHash()] = atv.containingBlock;

  for (const auto& b : atv.context) {
    block_index_[b.getShortHash()] = b;
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
                            PopData& popData,
                            AltTree& tree,
                            ValidationState& state) {
  bool ret = tree.acceptBlock(hack_block, state);
  assert(ret);

  // apply vbk_context
  for (const auto& b : popData.vbk_context) {
    ret = tree.vbk().acceptBlock(b, state);
    assert(ret);
  }

  auto genesis_height = tree.vbk().getParams().getGenesisBlock().height;
  auto settlement_interval =
      tree.vbk().getParams().getEndorsementSettlementInterval();
  // check VTB endorsements
  for (auto it = popData.vtbs.begin(); it != popData.vtbs.end();) {
    VTB& vtb = *it;
    auto* containing_block_index =
        tree.vbk().getBlockIndex(vtb.containingBlock.getHash());

    auto start_height = std::max(
        genesis_height, containing_block_index->height - settlement_interval);

    auto endorsement = BtcEndorsement::fromContainer(vtb);
    Chain<BlockIndex<VbkBlock>> chain(start_height, containing_block_index);
    auto duplicate =
        chain.findBlockContainingEndorsement(endorsement, settlement_interval);

    // invalid vtb
    if (duplicate) {
      // remove from storage
      stored_vtbs_.erase(vtb.getId());

      it = popData.vtbs.erase(it);
      continue;
    }
    ++it;
  }

  for (const auto& b : popData.vbk_context) {
    tree.vbk().invalidateBlockByHash(b.getHash());
  }

  AltPayloads payloads;
  payloads.popData = popData;
  payloads.containingBlock = hack_block;

  // find endorsed block
  auto endorsed_hash = hasher(popData.atv.transaction.publicationData.header);
  auto endorsed_block_index = tree.getBlockIndex(endorsed_hash);
  if (!endorsed_block_index) {
    return false;
  }
  payloads.endorsed = *endorsed_block_index->header;

  if (!tree.addPayloads(hack_block, {payloads}, state)) {
    stored_atvs_.erase(popData.atv.getId());
    return false;
  }

  return true;
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
    auto pair = std::make_pair(BtcEndorsement::getId(vtbs[i]), vtbs[i]);
    // clear contex
    pair.second.context.clear();

    stored_vtbs_.insert(pair);
  }

  return true;
}

std::vector<PopData> MemPool::getPop(const AltBlock& current_block,
                                     AltTree& tree,
                                     ValidationState& state) {
  bool ret = tree.setState(current_block.getHash(), state);
  (void)ret;
  assert(ret);

  AltBlock hack_block;
  hack_block.previousBlock = current_block.getHash();
  hack_block.timestamp = current_block.timestamp + 1;
  hack_block.height = current_block.height + 1;

  std::vector<std::pair<ATV::id_t, ATV>> sorted_atvs(stored_atvs_.size());
  std::copy(stored_atvs_.begin(), stored_atvs_.end(), sorted_atvs.begin());

  auto atv_comparator = [](const std::pair<ATV::id_t, ATV>& el1,
                           const std::pair<ATV::id_t, ATV>& el2) -> bool {
    return el1.second.containingBlock.height <
           el2.second.containingBlock.height;
  };
  std::sort(sorted_atvs.begin(), sorted_atvs.end(), atv_comparator);

  std::vector<PopData> v_popData;
  for (const auto& el : sorted_atvs) {
    auto& atv = el.second;
    PopData popData;
    VbkBlock first_block =
        !atv.context.empty() ? atv.context[0] : atv.containingBlock;

    if (!checkConnectivityWithTree(first_block, tree.vbk())) {
      if (fillContext(first_block, popData.vbk_context, tree)) {
        fillVTBs(popData.vtbs, popData.vbk_context);
        popData.atv = atv;
        popData.hasAtv = true;
        if (applyPayloads(hack_block, popData, tree, state)) {
          v_popData.push_back(popData);
        }
      }
    } else {
      popData.atv = atv;
      popData.hasAtv = true;
      if (applyPayloads(hack_block, popData, tree, state)) {
        v_popData.push_back(popData);
      }
    }
  }

  // clear tree from temp endorsement
  tree.invalidateBlockByHash(hack_block.getHash());

  return v_popData;
}

void MemPool::removePayloads(const std::vector<PopData>& v_popData) {
  for (const auto& tx : v_popData) {
    // clear context
    for (const auto& b : tx.vbk_context) {
      block_index_.erase(b.getShortHash());
    }

    // clear atv
    if (tx.hasAtv) {
      stored_atvs_.erase(tx.atv.getId());
    }

    // clear vtbs
    for (const auto& vtb : tx.vtbs) {
      stored_vtbs_.erase(BtcEndorsement::getId(vtb));
    }
  }
}

}  // namespace altintegration
