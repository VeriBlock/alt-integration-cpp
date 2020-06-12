// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

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

std::vector<std::pair<ATV::id_t, std::shared_ptr<ATV>>> getSortedATVs(
    const MemPool::atv_map_t& map) {
  std::vector<std::pair<ATV::id_t, std::shared_ptr<ATV>>> sorted_atvs(
      map.begin(), map.end());

  auto atv_comparator =
      [](const std::pair<ATV::id_t, std::shared_ptr<ATV>>& el1,
         const std::pair<ATV::id_t, std::shared_ptr<ATV>>& el2) -> bool {
    return el1.second->containingBlock.height <
           el2.second->containingBlock.height;
  };

  std::sort(sorted_atvs.begin(), sorted_atvs.end(), atv_comparator);
  return sorted_atvs;
}

}  // namespace

bool MemPool::fillContext(VbkBlock first_block,
                          std::vector<VbkBlock>& context,
                          AltTree& tree) {
  while (!checkConnectivityWithTree(first_block, tree.vbk())) {
    auto el = vbkblocks_.find(first_block.previousBlock);
    if (el != vbkblocks_.end()) {
      first_block = *el->second;
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
                       const std::vector<VbkBlock>& vbk_context) {
  for (const auto& b : vbk_context) {
    for (auto& vtb : stored_vtbs_) {
      if (vtb.second->containingBlock == b) {
        vtbs.push_back(*vtb.second);
      }
    }
  }
}

bool MemPool::applyPayloads(const AltBlock& hack_block,
                            PopData& popdata,
                            AltTree& tree,
                            ValidationState& state) {
  bool ret = tree.acceptBlock(hack_block, state);
  VBK_ASSERT(ret);

  // apply vbk_context
  for (const auto& b : popdata.vbk_context) {
    ret = tree.vbk().acceptBlock(b, state);
    VBK_ASSERT(ret);
  }

  auto genesis_height = tree.vbk().getParams().getGenesisBlock().height;
  auto settlement_interval =
      tree.vbk().getParams().getEndorsementSettlementInterval();
  // check VTB endorsements
  for (auto it = popdata.vtbs.begin(); it != popdata.vtbs.end();) {
    VTB& vtb = *it;
    auto* containing_block_index =
        tree.vbk().getBlockIndex(vtb.containingBlock.getHash());
    if (!containing_block_index) {
      throw std::logic_error("Mempool: containing block is not found: " +
                             HexStr(vtb.containingBlock.getHash()));
    }

    auto start_height = (std::max)(
        genesis_height, containing_block_index->height - settlement_interval);

    auto endorsement = VbkEndorsement::fromContainer(vtb);
    Chain<BlockIndex<VbkBlock>> chain(start_height, containing_block_index);
    auto duplicate =
        chain.findBlockContainingEndorsement(endorsement, settlement_interval);

    // invalid vtb
    if (duplicate) {
      // remove from storage
      stored_vtbs_.erase(vtb.getId());

      it = popdata.vtbs.erase(it);
      continue;
    }
    ++it;
  }

  for (const auto& b : popdata.vbk_context) {
    tree.vbk().removeSubtree(b.getHash());
  }

  AltPayloads payloads;
  payloads.popData = popdata;
  payloads.containingBlock = hack_block;

  // find endorsed block
  auto endorsed_hash = hasher(popdata.atv.transaction.publicationData.header);
  auto endorsed_block_index = tree.getBlockIndex(endorsed_hash);
  if (!endorsed_block_index) {
    return false;
  }
  payloads.endorsed = *endorsed_block_index->header;

  if (!tree.validatePayloads(hack_block.getHash(), payloads, state)) {
    stored_atvs_.erase(popdata.atv.getId());
    return false;
  }

  return true;
}

bool MemPool::submitATV(const std::vector<ATV>& atvs, ValidationState& state) {
  for (size_t i = 0; i < atvs.size(); ++i) {
    if (!submit<ATV>(atvs[i], state)) {
      return false;
    }
  }

  return true;
}

bool MemPool::submitVTB(const std::vector<VTB>& vtbs, ValidationState& state) {
  for (size_t i = 0; i < vtbs.size(); ++i) {
    if (!submit<VTB>(vtbs[i], state)) {
      return false;
    }
  }

  return true;
}

std::vector<PopData> MemPool::getPop(AltTree& tree) {
  ValidationState state;

  auto& tip = *tree.getBestChain().tip();
  AltBlock hack_block;
  hack_block.hash = std::vector<uint8_t>(32, 0);
  hack_block.previousBlock = tip.getHash();
  hack_block.timestamp = tip.getBlockTime() + 1;
  hack_block.height = tip.height + 1;

  auto sorted_atvs = getSortedATVs(stored_atvs_);

  std::vector<PopData> popTxs;
  for (size_t i = 0;
       i < sorted_atvs.size() && i < tree.getParams().getMaxPopDataPerBlock();
       ++i) {
    auto& atv = sorted_atvs[i].second;
    PopData popTx;
    VbkBlock first_block =
        !atv->context.empty() ? atv->context[0] : atv->containingBlock;

    if (!checkConnectivityWithTree(first_block, tree.vbk())) {
      if (fillContext(first_block, popTx.vbk_context, tree)) {
        fillVTBs(popTx.vtbs, popTx.vbk_context);
        popTx.atv = *atv;
        popTx.hasAtv = true;
        if (applyPayloads(hack_block, popTx, tree, state)) {
          popTxs.push_back(popTx);
        }
      }
    } else {
      popTx.atv = *atv;
      popTx.hasAtv = true;
      if (applyPayloads(hack_block, popTx, tree, state)) {
        popTxs.push_back(popTx);
      }
    }
  }

  // clear tree from temp endorsement
  tree.removeSubtree(hack_block.getHash());

  return popTxs;
}

void MemPool::removePayloads(const std::vector<PopData>& PopDatas) {
  for (const auto& tx : PopDatas) {
    // clear context
    for (const auto& b : tx.vbk_context) {
      vbkblocks_.erase(b.getShortHash());
    }

    // clear atv
    if (tx.hasAtv) {
      stored_atvs_.erase(tx.atv.getId());
    }

    // clear vtbs
    for (const auto& vtb : tx.vtbs) {
      stored_vtbs_.erase(vtb.getId());
    }
  }
}

MemPool::VbkBlockPayloads& MemPool::touchVbkBlock(const VbkBlock& block) {
  auto hash = block.getShortHash();
  auto it = vbkplds_.find(hash);
  if (it != vbkplds_.end()) {
    return *it->second;
  }

  auto pl = std::make_shared<VbkBlockPayloads>();
  vbkplds_[hash] = pl;

  vbkblocks_[hash] = std::make_shared<VbkBlock>(block);

  return *pl;
}

template <>
bool MemPool::submit(const ATV& atv, ValidationState& state) {
  if (!checkATV(atv, state, *alt_chain_params_, *vbk_chain_params_)) {
    return state.Invalid("pop-mempool-submit-atv");
  }

  for (const auto& b : atv.context) {
    touchVbkBlock(b);
  }

  auto& pl = touchVbkBlock(atv.containingBlock);

  auto atvid = atv.getId();
  auto pair = std::make_pair(atvid, std::make_shared<ATV>(atv));
  // clear context
  pair.second->context.clear();

  // store atv id in containing block index
  pl.atvs.insert(atvid);
  stored_atvs_.insert(pair);

  return true;
}

template <>
bool MemPool::submit(const VTB& vtb, ValidationState& state) {
  if (!checkVTB(vtb, state, *vbk_chain_params_, *btc_chain_params_)) {
    return state.Invalid("pop-mempool-submit-vtb");
  }

  for (const auto& b : vtb.context) {
    touchVbkBlock(b);
  }

  auto& pl = touchVbkBlock(vtb.containingBlock);

  auto vtbid = vtb.getId();
  auto pair = std::make_pair(vtbid, std::make_shared<VTB>(vtb));

  // clear context
  pair.second->context.clear();

  pl.vtbs.insert(vtbid);
  stored_vtbs_.insert(pair);

  return true;
}

}  // namespace altintegration
