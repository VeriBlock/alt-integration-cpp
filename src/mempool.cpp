// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/mempool.hpp"

#include <deque>
#include <veriblock/reversed_range.hpp>

#include "veriblock/entities/vbkfullblock.hpp"
#include "veriblock/stateless_validation.hpp"

namespace altintegration {

PopData MemPool::getPop(AltTree& tree) {
  PopData ret;
  // size in bytes of pop data added to
  size_t popSize = 0;

  // sorted array of VBK blocks (ascending order)
  using P = std::pair<VbkBlock::id_t, std::shared_ptr<VbkPayloadsRelations>>;
  std::vector<P> blocks(relations_.begin(), relations_.end());
  std::sort(blocks.begin(), blocks.end(), [](const P& a, const P& b) {
    return a.second->header.height < b.second->header.height;
  });

  for (const P& block : blocks) {
    PopData pop = block.second->toPopData();
    size_t estimated = pop.estimateSize();
    if (popSize + estimated <= tree.getParams().getMaxPopDataSize()) {
      // include this pop data into a block
      popSize += estimated;
      ret.mergeFrom(pop);
      continue;
    }

    // we are over max size
    break;
  }

  tree.filterInvalidPayloads(ret);
  return ret;
}

void MemPool::removePayloads(const PopData& pop) {
  // clear context
  for (const auto& b : pop.context) {
    auto hash = b.getId();
    vbkblocks_.erase(hash);
    relations_.erase(hash);
    removed_vbk_blocks.insert(hash);
  }

  // clear vtbs
  for (const auto& vtb : pop.vtbs) {
    auto vtb_id = vtb.getId();
    stored_vtbs_.erase(vtb_id);
    removed_vtbs.insert(vtb_id);
  }

  // clear atvs
  for (const auto& atv : pop.atvs) {
    auto atv_id = atv.getId();
    stored_atvs_.erase(atv_id);
    removed_atvs.insert(atv_id);
  }
}

MemPool::VbkPayloadsRelations& MemPool::touchVbkBlock(const VbkBlock& block,
                                                      VbkBlock::id_t block_id) {
  if (block_id == VbkBlock::id_t()) {
    block_id = block.getId();
  }

  vbkblocks_[block_id] = std::make_shared<VbkBlock>(block);

  auto& val = relations_[block_id];
  if (val == nullptr) {
    val = std::make_shared<VbkPayloadsRelations>(block);
  }

  return *val;
}

MempoolResult MemPool::submitAll(const PopData& pop, const AltTree& tree) {
  MempoolResult r;
  for (const VbkBlock& block : pop.context) {
    ValidationState state;
    submit<VbkBlock>(block, tree, state);
    r.context.emplace_back(block.getId(), state);
  }

  for (const VTB& vtb : pop.vtbs) {
    ValidationState state;
    submit<VTB>(vtb, tree, state);
    r.vtbs.emplace_back(vtb.getId(), state);
  }

  for (const ATV& atv : pop.atvs) {
    ValidationState state;
    submit<ATV>(atv, tree, state);
    r.atvs.emplace_back(atv.getId(), state);
  }

  return r;
}

template <>
bool MemPool::submit(const ATV& atv,
                     const AltTree& tree,
                     ValidationState& state) {
  // stateless validation
  if (!checkATV(atv, state, *alt_chain_params_, *vbk_chain_params_)) {
    return state.Invalid("pop-mempool-submit-atv-stateless");
  }

  // stateful validation
  auto window = alt_chain_params_->getEndorsementSettlementInterval();
  auto duplicate = findBlockContainingEndorsement(
      tree.getBestChain(), tree.getBestChain().tip(), atv.getId(), window);
  if (duplicate) {
    return state.Invalid(
        "pop-mempool-submit-atv-duplicate",
        fmt::sprintf("ATV=%s already added to active chain in block %s",
                     atv.getId().toHex(),
                     duplicate->toShortPrettyString()));
  }

  for (const auto& b : atv.context) {
    auto b_id = b.getId();
    if (removed_vbk_blocks.count(b_id) == 0) {
      touchVbkBlock(b, b_id);
    }
  }

  auto atv_id = atv.getId();
  if (removed_atvs.count(atv_id) == 0) {
    auto& rel = touchVbkBlock(atv.containingBlock);
    auto atvptr = std::make_shared<ATV>(atv);
    auto pair = std::make_pair(atv_id, atvptr);
    rel.atvs.push_back(atvptr);

    // clear context
    pair.second->context.clear();

    // store atv id in containing block index
    stored_atvs_.insert(pair);
  }

  return true;
}

template <>
bool MemPool::submit(const VTB& vtb,
                     const AltTree& tree,
                     ValidationState& state) {
  // stateless validation
  if (!checkVTB(vtb, state, *vbk_chain_params_, *btc_chain_params_)) {
    return state.Invalid("pop-mempool-submit-vtb-stateless");
  }

  // stateful validation
  auto window = vbk_chain_params_->getEndorsementSettlementInterval();
  auto duplicate =
      findBlockContainingEndorsement(tree.vbk().getBestChain(),
                                     tree.vbk().getBestChain().tip(),
                                     vtb.getId(),
                                     window);
  if (duplicate) {
    return state.Invalid(
        "pop-mempool-submit-vtb-duplicate",
        fmt::sprintf("VTB=%s already added to active chain in block %s",
                     vtb.getId().toHex(),
                     duplicate->toShortPrettyString()));
  }

  for (const auto& b : vtb.context) {
    auto b_id = b.getId();
    if (removed_vbk_blocks.count(b_id) == 0) {
      touchVbkBlock(b, b_id);
    }
  }

  auto vtb_id = vtb.getId();
  if (removed_vtbs.count(vtb_id) == 0) {
    auto& rel = touchVbkBlock(vtb.containingBlock);
    auto vtbptr = std::make_shared<VTB>(vtb);
    auto pair = std::make_pair(vtb_id, vtbptr);
    rel.vtbs.push_back(vtbptr);

    // clear context
    pair.second->context.clear();

    stored_vtbs_.insert(pair);
  }

  return true;
}

template <>
bool MemPool::submit(const VbkBlock& blk,
                     const AltTree& tree,
                     ValidationState& state) {
  // stateless validation
  if (!checkBlock(blk, state, *vbk_chain_params_)) {
    return state.Invalid("pop-mempool-submit-vbkblock-stateless");
  }

  // stateful validation
  if (tree.vbk().getBlockIndex(blk.getHash())) {
    // duplicate
    return state.Invalid("pop-mempool-submit-vbkblock-stateful");
  }

  auto blk_id = blk.getId();
  if (removed_vbk_blocks.count(blk_id) == 0) {
    touchVbkBlock(blk, blk_id);
  }

  return true;
}

}  // namespace altintegration
