// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <deque>
#include <veriblock/reversed_range.hpp>

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
    const std::vector<std::pair<VbkBlock::id_t,
                                std::shared_ptr<VbkPayloadsRelations>>>& blocks,
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

PopData MemPool::getPop() {
  // sorted array of VBK blocks (ascending order)
  using P = std::pair<VbkBlock::id_t, std::shared_ptr<VbkPayloadsRelations>>;
  std::vector<P> blocks(relations_.begin(), relations_.end());
  std::sort(blocks.begin(), blocks.end(), [](const P& a, const P& b) {
    return a.second->header->height < b.second->header->height;
  });

  PopData ret = generatePopData(blocks, mempool_tree_.alt().getParams());
  mempool_tree_.alt().filterInvalidPayloads(ret);
  return ret;
}

void MemPool::vacuum(const PopData& pop) {
  auto vbkblockids = make_idset(pop.context);
  auto vtbids = make_idset(pop.vtbs);
  auto atvids = make_idset(pop.atvs);

  // cascade removal of relation and stored payloads
  auto removeRelation = [&](decltype(relations_.begin()) it) {
    auto& rel = *it->second;
    vbkblocks_.erase(it->first);
    for (auto& vtb : rel.vtbs) {
      stored_vtbs_.erase(vtb->getId());
    }
    for (auto& atv : rel.atvs) {
      stored_atvs_.erase(atv->getId());
    }
    return relations_.erase(it);
  };

  for (auto it = relations_.begin(); it != relations_.end();) {
    auto blockHash = it->second->header->getHash();
    auto& vbk = mempool_tree_.vbk().getStableTree();
    auto* index = vbk.getBlockIndex(blockHash);
    auto* tip = vbk.getBestChain().tip();
    auto maxReorgBlocks = vbk.getParams().getMaxReorgBlocks();
    auto& rel = *it->second;

    bool tooOld = tip->getHeight() - maxReorgBlocks > rel.header->height;
    if (tooOld) {
      // VBK block is too old to be included or modified
      it = removeRelation(it);
      continue;
    }

    // cleanup stale VTBs
    for (auto vtbit = rel.vtbs.begin(); vtbit != rel.vtbs.end();) {
      auto& vtb = **vtbit;
      ValidationState state;
      auto id = vtb.getId();
      if (vtbids.count(id) > 0 ||
          !mempool_tree_.checkContextually(vtb, state)) {
        stored_vtbs_.erase(id);
        vtbit = rel.vtbs.erase(vtbit);
      } else {
        ++vtbit;
      }
    }

    // cleanup stale ATVs
    for (auto atvit = rel.atvs.begin(); atvit != rel.atvs.end();) {
      auto& atv = **atvit;
      auto id = atv.getId();
      ValidationState state;
      if (atvids.count(id) > 0 ||
          !mempool_tree_.checkContextually(atv, state)) {
        stored_atvs_.erase(id);
        atvit = rel.atvs.erase(atvit);
      } else {
        ++atvit;
      }
    }

    if (index != nullptr) {
      // VBK tree knows about this VBK block
      // does it know about stored VTBs?
      auto& v = index->getPayloadIds<VTB>();
      std::set<VTB::id_t> ids(v.begin(), v.end());
      // include vtbs that have just been included into new block
      std::transform(pop.vtbs.begin(),
                     pop.vtbs.end(),
                     std::inserter(ids, std::end(ids)),
                     get_id<VTB>);

      for (auto vtbit = rel.vtbs.begin(); vtbit != rel.vtbs.end();) {
        auto id = (*vtbit)->getId();
        // mempool contains VBK block, which already exists in VBK tree, and
        // we found a VTB which exists in that VBK block. we can remove VTB
        // from mempool
        vtbit = ids.count(id) ? rel.vtbs.erase(vtbit) : std::next(vtbit);
      }

      if (rel.empty()) {
        it = removeRelation(it);
        continue;
      }
    }

    // if header is recently added to new block or relation is empty, cleanup
    if (vbkblockids.count(rel.header->getId()) && rel.empty()) {
      it = removeRelation(it);
      continue;
    }

    ++it;
  }
}

void MemPool::removeAll(const PopData& pop) { vacuum(pop); }

VbkPayloadsRelations& MemPool::touchVbkPayloadRelation(
    const std::shared_ptr<VbkBlock>& block_ptr) {
  auto block_id = block_ptr->getId();

  vbkblocks_[block_id] = block_ptr;

  auto& val = relations_[block_id];
  if (val == nullptr) {
    val = std::make_shared<VbkPayloadsRelations>(block_ptr);
  }

  on_vbkblock_accepted.emit(*block_ptr);

  return *val;
}

namespace {

template <typename pop_t>
void process_submit(
    MemPool& memPool,
    const std::vector<pop_t>& payloads,
    std::vector<std::pair<typename pop_t::id_t, ValidationState>>& res) {
  for (const auto& p : payloads) {
    ValidationState state;
    memPool.submit<pop_t>(p, state);
    res.emplace_back(p.getId(), state);
  }
}

}  // namespace

MempoolResult MemPool::submitAll(const PopData& pop) {
  MempoolResult r;

  process_submit(*this, pop.context, r.context);
  process_submit(*this, pop.vtbs, r.vtbs);
  process_submit(*this, pop.atvs, r.atvs);

  return r;
}

void MemPool::clear() {
  mempool_tree_.clear();
  relations_.clear();
  vbkblocks_.clear();
  stored_vtbs_.clear();
  stored_atvs_.clear();
}

template <>
bool MemPool::submit(const ATV& atv,
                     ValidationState& state,
                     bool shouldDoContextualCheck) {
  // stateless validation
  if (!checkATV(atv, state, mempool_tree_.alt().getParams())) {
    return state.Invalid("pop-mempool-submit-atv-stateless");
  }

  // stateful validation
  if (shouldDoContextualCheck && !mempool_tree_.acceptATV(atv, state)) {
    return state.Invalid("pop-mempool-submit-atv-stateful");
  }

  auto& rel =
      touchVbkPayloadRelation(std::make_shared<VbkBlock>(atv.blockOfProof));
  auto atvptr = std::make_shared<ATV>(atv);
  auto pair = std::make_pair(atv.getId(), atvptr);
  rel.atvs.push_back(atvptr);

  // store atv id in containing block index
  stored_atvs_.insert(pair);

  on_atv_accepted.emit(atv);

  return true;
}

template <>
bool MemPool::submit(const VTB& vtb,
                     ValidationState& state,
                     bool shouldDoContextualCheck) {
  // stateless validation
  if (!checkVTB(vtb, state, mempool_tree_.btc().getStableTree().getParams())) {
    return state.Invalid("pop-mempool-submit-vtb-stateless");
  }

  // stateful validation
  if (shouldDoContextualCheck && !mempool_tree_.acceptVTB(vtb, state)) {
    return state.Invalid("pop-mempool-submit-vtb-stateful");
  }

  auto& rel =
      touchVbkPayloadRelation(std::make_shared<VbkBlock>(vtb.containingBlock));
  auto vtbptr = std::make_shared<VTB>(vtb);
  auto pair = std::make_pair(vtb.getId(), vtbptr);
  rel.vtbs.push_back(vtbptr);

  stored_vtbs_.insert(pair);

  on_vtb_accepted.emit(vtb);

  return true;
}

template <>
bool MemPool::submit(const VbkBlock& blk,
                     ValidationState& state,
                     bool shouldDoContextualCheck) {
  // stateless validation
  if (!checkBlock(
          blk, state, mempool_tree_.vbk().getStableTree().getParams())) {
    return state.Invalid("pop-mempool-submit-vbkblock-stateless");
  }

  std::shared_ptr<VbkBlock> blk_ptr = std::make_shared<VbkBlock>(blk);

  if (shouldDoContextualCheck &&
      !mempool_tree_.acceptVbkBlock(blk_ptr, state)) {
    return state.Invalid("pop-mempool-submit-vbk-stateful");
  }

  // stateful validation
  if (!shouldDoContextualCheck ||
      !mempool_tree_.vbk().getStableTree().getBlockIndex(blk.getHash())) {
    // duplicate
    touchVbkPayloadRelation(blk_ptr);
  }

  return true;
}

template <>
const MemPool::payload_map<VbkBlock>& MemPool::getMap() const {
  return vbkblocks_;
}

template <>
const MemPool::payload_map<ATV>& MemPool::getMap() const {
  return stored_atvs_;
}

template <>
const MemPool::payload_map<VTB>& MemPool::getMap() const {
  return stored_vtbs_;
}

template <>
signals::Signal<void(const ATV&)>& MemPool::getSignal() {
  return on_atv_accepted;
}

template <>
signals::Signal<void(const VTB&)>& MemPool::getSignal() {
  return on_vtb_accepted;
}

template <>
signals::Signal<void(const VbkBlock&)>& MemPool::getSignal() {
  return on_vbkblock_accepted;
}

}  // namespace altintegration
