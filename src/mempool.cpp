// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <deque>
#include <veriblock/reversed_range.hpp>

#include "veriblock/entities/vbkfullblock.hpp"
#include "veriblock/mempool.hpp"
#include "veriblock/stateless_validation.hpp"

namespace altintegration {

namespace {

template <typename pop_t>
bool process_cut_payloads(std::vector<pop_t>& payloads, size_t& current_size) {
  if (!payloads.empty()) {
    auto& p = payloads.back();
    current_size -= p.toVbkEncoding().size();
    payloads.pop_back();
    return true;
  }

  return false;
}

size_t cutPopData(PopData& popData, size_t current_size) {
  // first remove vtb
  if (process_cut_payloads(popData.vtbs, current_size)) {
    return current_size;
  }
  // second remove atv
  if (process_cut_payloads(popData.atvs, current_size)) {
    return current_size;
  }
  // third remove vbk blocks
  if (process_cut_payloads(popData.context, current_size)) {
    return current_size;
  }

  return current_size;
}

PopData generatePopData(
    const std::vector<
        std::pair<VbkBlock::id_t,
                  std::shared_ptr<MemPool::VbkPayloadsRelations>>>& blocks,
    const AltChainParams& params) {
  PopData ret;
  // size in bytes of pop data added to
  size_t popSize = 0;

  for (const auto& block : blocks) {
    PopData pop = block.second->toPopData();
    size_t estimated = pop.estimateSize();

    while (popSize + estimated > params.getMaxPopDataSize() && !pop.empty()) {
      estimated = cutPopData(pop, estimated);
    }

    if (popSize + estimated > params.getMaxPopDataSize() || pop.empty()) {
      continue;
    }

    popSize += estimated;
    ret.mergeFrom(pop);

    if (popSize > params.getMaxPopDataSize()) {
      break;
    }
  }

  return ret;
}

}  // namespace

PopData MemPool::VbkPayloadsRelations::toPopData() const {
  PopData pop;
  pop.context.push_back(*header);
  for (const auto& vtb : vtbs) {
    pop.vtbs.push_back(*vtb);
  }

  for (const auto& atv : atvs) {
    pop.atvs.push_back(*atv);
  }

  // TODO: we might want to sort VTBs in ascending order of their
  // blockOfProofs to guarantee that within a single block they all are
  // connected.

  return pop;
}

void MemPool::VbkPayloadsRelations::removeVTB(const VTB::id_t& vtb_id) {
  auto it = std::find_if(
      vtbs.begin(), vtbs.end(), [&vtb_id](const std::shared_ptr<VTB>& vtb) {
        return vtb->getId() == vtb_id;
      });

  if (it != vtbs.end()) {
    vtbs.erase(it);
  }
}

void MemPool::VbkPayloadsRelations::removeATV(const ATV::id_t& atv_id) {
  auto it = std::find_if(
      atvs.begin(), atvs.end(), [&atv_id](const std::shared_ptr<ATV>& atv) {
        return atv->getId() == atv_id;
      });

  if (it != atvs.end()) {
    atvs.erase(it);
  }
}

PopData MemPool::getPop(AltTree& tree) {
  // sorted array of VBK blocks (ascending order)
  using P = std::pair<VbkBlock::id_t, std::shared_ptr<VbkPayloadsRelations>>;
  std::vector<P> blocks(relations_.begin(), relations_.end());
  std::sort(blocks.begin(), blocks.end(), [](const P& a, const P& b) {
    return a.second->header->height < b.second->header->height;
  });

  PopData ret = generatePopData(blocks, tree.getParams());

  tree.filterInvalidPayloads(ret);
  return ret;
}

void MemPool::filterVbkBlocks(const AltTree& tree) {
  for (auto rel_it = relations_.begin(); rel_it != relations_.end();) {
    auto blockHash = rel_it->second->header->getHash();
    auto* index = tree.vbk().getBlockIndex(blockHash);

    if (index != nullptr && rel_it->second->empty()) {
      vbkblocks_.erase(rel_it->first);
      rel_it = relations_.erase(rel_it);
      continue;
    }
    ++rel_it;
  }
}

void MemPool::removePayloads(const PopData& pop, const AltTree& tree) {
  // clear vtbs
  for (const auto& vtb : pop.vtbs) {
    auto vtb_id = vtb.getId();

    auto relation_it = relations_.find(vtb.containingBlock.getId());
    if (relation_it == relations_.end()) {
      continue;
    }

    relation_it->second->removeVTB(vtb_id);
    stored_vtbs_.erase(vtb_id);
  }

  // clear atvs
  for (const auto& atv : pop.atvs) {
    auto atv_id = atv.getId();

    auto relation_it = relations_.find(atv.blockOfProof.getId());
    if (relation_it == relations_.end()) {
      continue;
    }

    relation_it->second->removeATV(atv_id);
    stored_atvs_.erase(atv_id);
  }

  // clear context
  for (const auto& b : pop.context) {
    auto hash = b.getId();
    auto relation_it = relations_.find(hash);
    if (relation_it == relations_.end()) {
      continue;
    }

    if (relation_it->second->empty()) {
      vbkblocks_.erase(hash);
      relations_.erase(relation_it);
    }
  }

  filterVbkBlocks(tree);
}

MemPool::VbkPayloadsRelations& MemPool::touchVbkBlock(const VbkBlock& block,
                                                      VbkBlock::id_t block_id) {
  if (block_id == VbkBlock::id_t()) {
    block_id = block.getId();
  }

  std::shared_ptr<VbkBlock> vbk_block = std::make_shared<VbkBlock>(block);

  vbkblocks_[block_id] = vbk_block;

  auto& val = relations_[block_id];
  if (val == nullptr) {
    val = std::make_shared<VbkPayloadsRelations>(vbk_block);
  }

  return *val;
}

namespace {

template <typename pop_t>
void process_submit(
    MemPool& memPool,
    const std::vector<pop_t>& payloads,
    const AltTree& tree,
    std::vector<std::pair<typename pop_t::id_t, ValidationState>>& res) {
  for (const auto& p : payloads) {
    ValidationState state;
    memPool.submit<pop_t>(p, tree, state);
    res.emplace_back(p.getId(), state);
  }
}

}  // namespace

MempoolResult MemPool::submitAll(const PopData& pop, const AltTree& tree) {
  MempoolResult r;

  process_submit(*this, pop.context, tree, r.context);
  process_submit(*this, pop.vtbs, tree, r.vtbs);
  process_submit(*this, pop.atvs, tree, r.atvs);

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
  int32_t window = alt_chain_params_->getEndorsementSettlementInterval();
  auto duplicate = findBlockContainingEndorsement(
      tree.getBestChain(), tree.getBestChain().tip(), atv.getId(), window);
  if (duplicate) {
    return state.Invalid(
        "pop-mempool-submit-atv-duplicate",
        fmt::sprintf("ATV=%s already added to active chain in block %s",
                     atv.getId().toHex(),
                     duplicate->toShortPrettyString()));
  }

  std::vector<uint8_t> endorsed_hash =
      tree.getParams().getHash(atv.transaction.publicationData.header);
  auto* endorsed_index = tree.getBlockIndex(endorsed_hash);
  if (endorsed_index != nullptr) {
    auto* tip = tree.getBestChain().tip();
    assert(tip != nullptr && "block tree is not bootstrapped");

    if (tip && (tip->getHeight() - endorsed_index->getHeight() + 1 > window)) {
      return state.Invalid("pop-mempool-submit-atv-expired",
                           fmt::sprintf("ATV=%s expired %s",
                                        atv.getId().toHex(),
                                        endorsed_index->toShortPrettyString()));
    }
  }

  for (const auto& b : atv.context) {
    // stateful validation
    if (!tree.vbk().getBlockIndex(b.getHash())) {
      touchVbkBlock(b, b.getId());
    }
  }

  auto& rel = touchVbkBlock(atv.blockOfProof);
  auto atvptr = std::make_shared<ATV>(atv);
  auto pair = std::make_pair(atv.getId(), atvptr);
  rel.atvs.push_back(atvptr);

  // clear context
  pair.second->context.clear();

  // store atv id in containing block index
  stored_atvs_.insert(pair);

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
  auto* containing = tree.vbk().getBlockIndex(vtb.containingBlock.getHash());
  int32_t window = vbk_chain_params_->getEndorsementSettlementInterval();
  auto duplicate = findBlockContainingEndorsement(
      tree.vbk().getBestChain(),
      // if containing exists on chain, then search for duplicates starting from
      // containing, else search starting from tip
      (containing ? containing : tree.vbk().getBestChain().tip()),
      vtb.getId(),
      window);
  if (duplicate) {
    return state.Invalid(
        "pop-mempool-submit-vtb-duplicate",
        fmt::sprintf("VTB=%s already added to active chain in block %s",
                     vtb.getId().toHex(),
                     duplicate->toShortPrettyString()));
  }

  if (vtb.containingBlock.height - vtb.transaction.publishedBlock.height >
      window) {
    return state.Invalid(
        "pop-mempool-submit-vtb-expired",
        fmt::sprintf("VTB=%s expired %s",
                     vtb.getId().toHex(),
                     vtb.transaction.publishedBlock.toPrettyString()));
  }

  for (const auto& b : vtb.context) {
    // stateful validation
    if (!tree.vbk().getBlockIndex(b.getHash())) {
      touchVbkBlock(b, b.getId());
    }
  }

  auto& rel = touchVbkBlock(vtb.containingBlock);
  auto vtbptr = std::make_shared<VTB>(vtb);
  auto pair = std::make_pair(vtb.getId(), vtbptr);
  rel.vtbs.push_back(vtbptr);

  // clear context
  pair.second->context.clear();

  stored_vtbs_.insert(pair);

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

  touchVbkBlock(blk, blk.getId());

  return true;
}

}  // namespace altintegration
