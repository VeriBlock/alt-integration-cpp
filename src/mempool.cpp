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

  // we sort VTBs in ascending order of their containing VBK blocks to guarantee
  // that within a single block they all are connected.
  std::sort(pop.vtbs.begin(), pop.vtbs.end(), [](const VTB& a, const VTB& b) {
    return a.containingBlock.height < b.containingBlock.height;
  });

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

PopData MemPool::getPop() {
  // sorted array of VBK blocks (ascending order)
  using P = std::pair<VbkBlock::id_t, std::shared_ptr<VbkPayloadsRelations>>;
  std::vector<P> blocks(relations_.begin(), relations_.end());
  std::sort(blocks.begin(), blocks.end(), [](const P& a, const P& b) {
    return a.second->header->height < b.second->header->height;
  });

  PopData ret = generatePopData(blocks, tree_->getParams());
  tree_->filterInvalidPayloads(ret);
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
    auto& vbk = tree_->vbk();
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
      if (vtbids.count(id) > 0 || !checkContextually(vtb, state)) {
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
      if (atvids.count(id) > 0 || !checkContextually(atv, state)) {
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

void MemPool::removePayloads(const PopData& pop) { vacuum(pop); }

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

  on_vbkblock_accepted.emit(block);

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
  if (!checkATV(atv, state, tree_->getParams())) {
    return state.Invalid("pop-mempool-submit-atv-stateless");
  }

  // stateful validation
  if (shouldDoContextualCheck && !checkContextually(atv, state)) {
    return state.Invalid("pop-mempool-submit-atv-stateful");
  }

  auto& rel = touchVbkBlock(atv.blockOfProof);
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
  auto& vbk = tree_->vbk();
  // stateless validation
  if (!checkVTB(vtb, state, vbk.btc().getParams())) {
    return state.Invalid("pop-mempool-submit-vtb-stateless");
  }

  // stateful validation
  if (shouldDoContextualCheck && !checkContextually(vtb, state)) {
    return state.Invalid("pop-mempool-submit-vtb-stateful");
  }

  auto& rel = touchVbkBlock(vtb.containingBlock);
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
  if (!checkBlock(blk, state, tree_->vbk().getParams())) {
    return state.Invalid("pop-mempool-submit-vbkblock-stateless");
  }

  if (shouldDoContextualCheck && !checkContextually(blk, state)) {
    return state.Invalid("pop-mempool-submit-vbk-stateful");
  }

  // stateful validation
  if (!shouldDoContextualCheck || !tree_->vbk().getBlockIndex(blk.getHash())) {
    // duplicate
    touchVbkBlock(blk, blk.getId());
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

template <>
bool MemPool::checkContextually<VTB>(const VTB& vtb, ValidationState& state) {
  auto& vbk = tree_->vbk();
  auto* containing = vbk.getBlockIndex(vtb.containingBlock.getHash());
  int32_t window = vbk.getParams().getEndorsementSettlementInterval();
  auto duplicate = findBlockContainingEndorsement(
      vbk.getBestChain(),
      // if containing exists on chain, then search for duplicates starting from
      // containing, else search starting from tip
      (containing ? containing : vbk.getBestChain().tip()),
      vtb.getId(),
      window);
  if (duplicate) {
    return state.Invalid(
        "vtb-duplicate",
        fmt::sprintf("VTB=%s already added to active chain in block %s",
                     vtb.getId().toHex(),
                     duplicate->toShortPrettyString()));
  }

  if (vtb.containingBlock.height - vtb.transaction.publishedBlock.height >
      window) {
    return state.Invalid(
        "vtb-expired",
        fmt::sprintf("VTB=%s expired %s",
                     vtb.getId().toHex(),
                     vtb.transaction.publishedBlock.toPrettyString()));
  }

  return true;
}

template <>
bool MemPool::checkContextually<ATV>(const ATV& atv, ValidationState& state) {
  // stateful validation
  int32_t window = tree_->getParams().getEndorsementSettlementInterval();
  auto duplicate = findBlockContainingEndorsement(
      tree_->getBestChain(), tree_->getBestChain().tip(), atv.getId(), window);
  if (duplicate) {
    return state.Invalid(
        "atv-duplicate",
        fmt::sprintf("ATV=%s already added to active chain in block %s",
                     atv.getId().toHex(),
                     duplicate->toShortPrettyString()));
  }

  auto endorsed_hash =
      tree_->getParams().getHash(atv.transaction.publicationData.header);
  auto* endorsed_index = tree_->getBlockIndex(endorsed_hash);
  if (endorsed_index != nullptr) {
    auto* tip = tree_->getBestChain().tip();
    assert(tip != nullptr && "block tree is not bootstrapped");

    if (tip && (tip->getHeight() - endorsed_index->getHeight() + 1 > window)) {
      return state.Invalid("atv-expired",
                           fmt::sprintf("ATV=%s expired %s",
                                        atv.getId().toHex(),
                                        endorsed_index->toShortPrettyString()));
    }
  }

  return true;
}

template <>
bool MemPool::checkContextually<VbkBlock>(const VbkBlock& blk,
                                          ValidationState& state) {
  if (tree_->vbk().getBlockIndex(blk.previousBlock) == nullptr &&
      get<VbkBlock>(blk.previousBlock) == nullptr) {
    return state.Invalid(
        "bad-prev",
        fmt::sprintf("Block=%s does not connect to known VBK tree",
                     blk.toPrettyString()));
  }

  return true;
}

}  // namespace altintegration
