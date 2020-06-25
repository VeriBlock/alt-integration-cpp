// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/mempool.hpp"

#include <deque>
#include <veriblock/reversed_range.hpp>

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
  std::vector<VbkBlock> temp;
  temp.push_back(first_block);

  while (!checkConnectivityWithTree(first_block, tree.vbk())) {
    auto el = vbkblocks_.find(first_block.previousBlock);
    if (el != vbkblocks_.end()) {
      first_block = *el->second;
      temp.push_back(first_block);
    } else {
      return false;
    }
  }

  // reverse context
  std::reverse(temp.begin(), temp.end());

  // apply blocks to the tree
  ValidationState state;
  for (const auto& b : temp) {
    if (!tree.vbk().acceptBlock(b, state)) {
      return false;
    }
  }

  // add to the context vector
  context.insert(context.end(), temp.begin(), temp.end());

  return true;
}

bool MemPool::applyATV(const ATV& atv,
                       const AltBlock& hack_block,
                       AltTree& tree,
                       ValidationState& state) {
  bool ret = tree.acceptBlock(hack_block, state);
  VBK_ASSERT(ret);

  PopData temp;
  temp.atvs = {atv};

  if (!tree.validatePayloads(hack_block.getHash(), temp, state)) {
    return false;
  }

  return true;
}

bool MemPool::applyVTB(const VTB& vtb,
                       const AltBlock& hack_block,
                       AltTree& tree,
                       ValidationState& state) {
  bool ret = tree.acceptBlock(hack_block, state);
  VBK_ASSERT(ret);

  PopData temp;
  temp.vtbs = {vtb};

  if (!tree.validatePayloads(hack_block.getHash(), temp, state)) {
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

PopData MemPool::getPop(AltTree& tree) {
  ValidationState state;

  auto& tip = *tree.getBestChain().tip();
  AltBlock hack_block;
  hack_block.hash = std::vector<uint8_t>(32, 0);
  hack_block.previousBlock = tip.getHash();
  hack_block.timestamp = tip.getBlockTime() + 1;
  hack_block.height = tip.height + 1;

  auto sorted_atvs = getSortedATVs(stored_atvs_);

  PopData popData;

  // fill atvs
  for (size_t i = 0;
       i < sorted_atvs.size() && i < tree.getParams().getMaxPopDataPerBlock();
       ++i) {
    auto& atv = sorted_atvs[i].second;
    VbkBlock first_block = atv->containingBlock;

    if (fillContext(first_block, popData.context, tree)) {
      if (applyATV(*atv, hack_block, tree, state)) {
        popData.atvs.push_back(*atv);
      } else {
        stored_atvs_.erase(atv->getId());
      }
    }
  }

  // fill vtbs
  for (const auto& b : popData.context) {
    for (auto it = stored_vtbs_.begin(); it != stored_vtbs_.end();) {
      if (it->second->containingBlock == b) {
        if (applyVTB(*it->second, hack_block, tree, state)) {
          popData.vtbs.push_back(*it->second);
        } else {
          it = stored_vtbs_.erase(it);
          continue;
        }
      }
      ++it;
    }
  }

  // clear tree from temp endorsement
  tree.removeSubtree(hack_block.getHash());
  // clear from the context blocks
  for (const auto& b : popData.context) {
    tree.vbk().removeSubtree(b.getHash());
  }

  return popData;
}

void MemPool::removePayloads(const PopData& popData) {
  // clear context
  for (const auto& b : popData.context) {
    vbkblocks_.erase(b.getId());
  }

  // clear atvs
  for (const auto& atv : popData.atvs) {
    stored_atvs_.erase(atv.getId());
  }

  // clear vtbs
  for (const auto& vtb : popData.vtbs) {
    stored_vtbs_.erase(vtb.getId());
  }
}

template <>
bool MemPool::submit(const ATV& atv, ValidationState& state) {
  if (!checkATV(atv, state, *alt_chain_params_, *vbk_chain_params_)) {
    return state.Invalid("pop-mempool-submit-atv");
  }

  for (const auto& b : atv.context) {
    vbkblocks_[b.getId()] = std::make_shared<VbkBlock>(b);
  }

  vbkblocks_[atv.containingBlock.getId()] =
      std::make_shared<VbkBlock>(atv.containingBlock);

  auto atvid = atv.getId();
  auto pair = std::make_pair(atvid, std::make_shared<ATV>(atv));
  // clear context
  pair.second->context.clear();

  // store atv id in containing block index
  stored_atvs_.insert(pair);

  return true;
}

template <>
bool MemPool::submit(const VTB& vtb, ValidationState& state) {
  if (!checkVTB(vtb, state, *vbk_chain_params_, *btc_chain_params_)) {
    return state.Invalid("pop-mempool-submit-vtb");
  }

  for (const auto& b : vtb.context) {
    vbkblocks_[b.getId()] = std::make_shared<VbkBlock>(b);
  }

  vbkblocks_[vtb.containingBlock.getId()] =
      std::make_shared<VbkBlock>(vtb.containingBlock);

  auto vtbid = vtb.getId();
  auto pair = std::make_pair(vtbid, std::make_shared<VTB>(vtb));

  // clear context
  pair.second->context.clear();

  stored_vtbs_.insert(pair);

  return true;
}

template <>
bool MemPool::submit(const VbkBlock& blk, ValidationState& state) {
  if (!checkBlock(blk, state, *vbk_chain_params_)) {
    return state.Invalid("pop-mempool-submit-vbkblock");
  }

  vbkblocks_[blk.getShortHash()] = std::make_shared<VbkBlock>(blk);

  return true;
}

}  // namespace altintegration
