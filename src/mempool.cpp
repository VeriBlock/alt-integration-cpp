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

PopData generatePopData(
    const std::vector<std::pair<VbkBlock::id_t,
                                std::shared_ptr<VbkPayloadsRelations>>>& blocks,
    const AltChainParams& params) {
  PopData ret;
  // size in bytes of pop data added to
  size_t popSize = 0;

  const auto& maxSize = params.getMaxPopDataSize();
  const auto& maxVbkBlocks = params.getMaxVbkBlocksInAltBlock();
  const auto& maxVTBs = params.getMaxVTBsInAltBlock();
  const auto& maxATVs = params.getMaxATVsInAltBlock();
  for (const auto& block : blocks) {
    // add VBK block if it fits
    auto& header = *block.second->header;
    if (ret.context.size() >= maxVbkBlocks ||
        popSize + header.estimateSize() >= maxSize) {
      // PopData is full
      break;
    }
    // add VBK block to connect underlying ATVs/VTBs
    ret.context.push_back(header);
    popSize += header.estimateSize();

    // try to fit ATVs
    auto& atvcandidates = block.second->atvs;
    for (const auto& atv : atvcandidates) {
      const auto estimate = atv->estimateSize();
      if (ret.atvs.size() >= maxATVs || popSize + estimate >= maxSize) {
        // do not consider this ATV, it does not fit
        continue;
      }

      // this ATV fits
      ret.atvs.push_back(*atv);
      popSize += estimate;
    }

    // try to fit VTBs
    auto& vtbcandidates = block.second->vtbs;
    for (const auto& vtb : vtbcandidates) {
      const auto estimate = vtb->estimateSize();
      if (ret.vtbs.size() >= maxVTBs || popSize + estimate >= maxSize) {
        // this VTB does not fit
        continue;
      }

      ret.vtbs.push_back(*vtb);
      popSize += estimate;
    }
  }

  VBK_ASSERT(ret.context.size() <= maxVbkBlocks);
  VBK_ASSERT(ret.vtbs.size() <= maxVTBs);
  VBK_ASSERT(ret.atvs.size() <= maxATVs);
  const auto estimate = ret.estimateSize();
  VBK_ASSERT_MSG(estimate <= maxSize, "estimate=%d, max=%d", estimate, maxSize);

  return ret;
}

}  // namespace

PopData MemPool::getPop() {
  // attempt to connect payloads
  tryConnectPayloads();

  // sorted array of VBK blocks (ascending order)
  using P = std::pair<VbkBlock::id_t, std::shared_ptr<VbkPayloadsRelations>>;
  std::vector<P> blocks(relations_.begin(), relations_.end());
  std::sort(blocks.begin(), blocks.end(), [](const P& a, const P& b) {
    return a.second->header->getHeight() < b.second->header->getHeight();
  });

  PopData ret = generatePopData(blocks, mempool_tree_.alt().getParams());
  mempool_tree_.alt().filterInvalidPayloads(ret);
  return ret;
}

void MemPool::cleanUp() {
  auto& vbk_tree = mempool_tree_.vbk().getStableTree();

  for (auto it = relations_.begin(); it != relations_.end();) {
    auto& rel = *it->second;
    auto* index = vbk_tree.getBlockIndex(it->second->header->getHash());

    auto* tip = vbk_tree.getBestChain().tip();
    VBK_ASSERT(tip);
    bool tooOld = tip->getHeight() - vbk_tree.getParams().getMaxReorgBlocks() >
                  rel.header->getHeight();

    // cleanup stale relations
    if (tooOld) {
      // remove ATVs
      for (const auto& atv : rel.atvs) {
        stored_atvs_.erase(atv->getId());
      }

      // remove VTBs
      for (const auto& vtb : rel.vtbs) {
        stored_vtbs_.erase(vtb->getId());
      }

      // remove vbk block
      vbkblocks_.erase(rel.header->getId());
      it = relations_.erase(it);

      continue;
    }

    // cleanup stale VTBs
    cleanupStale<VTB>(rel.vtbs,
                      [this](const VTB& v) { stored_vtbs_.erase(v.getId()); });

    // cleanup stale ATVs
    cleanupStale<ATV>(rel.atvs,
                      [this](const ATV& v) { stored_atvs_.erase(v.getId()); });

    if (index != nullptr && rel.empty()) {
      vbkblocks_.erase(rel.header->getId());
      it = relations_.erase(it);
      continue;
    }

    ++it;
  }

  // remove payloads from inFlight storage
  cleanupStale<VTB>(vtbs_in_flight_);
  cleanupStale<ATV>(atvs_in_flight_);
  cleanupStale<VbkBlock>(vbkblocks_in_flight_);

  mempool_tree_.cleanUp();

  VBK_ASSERT_MSG(relations_.size() == vbkblocks_.size(),
                 "Relations=%d, vbkblocks=%d",
                 relations_.size(),
                 vbkblocks_.size());
}

void MemPool::removeAll(const PopData& pop) {
  auto vbkblockids = make_idset(pop.context);
  auto vtbids = make_idset(pop.vtbs);
  auto atvids = make_idset(pop.atvs);

  for (auto it = relations_.begin(); it != relations_.end();) {
    auto& rel = *it->second;
    auto block_id = rel.header->getId();

    // remove ATVs
    for (auto atvit = rel.atvs.begin(); atvit != rel.atvs.end();) {
      auto id = (*atvit)->getId();
      atvit = atvids.count(id) != 0u
                  ? (stored_atvs_.erase(id), rel.atvs.erase(atvit))
                  : std::next(atvit);
    }

    // remove VTBs
    for (auto vtbit = rel.vtbs.begin(); vtbit != rel.vtbs.end();) {
      auto id = (*vtbit)->getId();
      vtbit = vtbids.count(id) != 0u
                  ? (stored_vtbs_.erase(id), rel.vtbs.erase(vtbit))
                  : std::next(vtbit);
    }

    // if header is recently added to new block or relation is empty, cleanup

    if ((vbkblockids.count(block_id) != 0u) && rel.empty()) {
      vbkblocks_.erase(block_id);
      it = relations_.erase(it);
      continue;
    }

    ++it;
  }

  this->cleanUp();

  // attempt to connect payloads
  tryConnectPayloads();
}

VbkPayloadsRelations& MemPool::getOrPutVbkRelation(
    const std::shared_ptr<VbkBlock>& block) {
  auto block_id = block->getId();
  vbkblocks_.insert({block_id, block});
  auto& val = relations_[block_id];
  if (val == nullptr) {
    val = std::make_shared<VbkPayloadsRelations>(block);
    on_vbkblock_accepted.emit(*block);
  }

  return *val;
}

void MemPool::clear() {
  mempool_tree_.clear();
  relations_.clear();
  vbkblocks_.clear();
  stored_vtbs_.clear();
  stored_atvs_.clear();
}

template <>
MemPool::SubmitResult MemPool::submit<ATV>(const std::shared_ptr<ATV>& atv,
                                           ValidationState& state) {
  // stateless validation
  if (!checkATV(*atv, state, mempool_tree_.alt().getParams())) {
    return {MemPool::FAILED_STATELESS, state.Invalid("atv-stateless")};
  }

  auto id = atv->getId();
  auto blockOfProof_ptr = std::make_shared<VbkBlock>(atv->blockOfProof);

  // stateful validation
  if (!mempool_tree_.acceptATV(*atv, blockOfProof_ptr, state)) {
    atvs_in_flight_[id] = atv;
    on_atv_accepted.emit(*atv);
    return {MemPool::FAILED_STATEFUL, state.Invalid("atv-stateful")};
  }

  VBK_LOG_DEBUG("[POP mempool] ATV=%s is connected", id.toHex());
  auto& rel = getOrPutVbkRelation(blockOfProof_ptr);
  rel.atvs.push_back(atv);
  makePayloadConnected<ATV>(atv);

  return true;
}

template <>
MemPool::SubmitResult MemPool::submit<VTB>(const std::shared_ptr<VTB>& vtb,
                                           ValidationState& state) {
  // stateless validation
  if (!checkVTB(*vtb, state, mempool_tree_.btc().getStableTree().getParams())) {
    return {FAILED_STATELESS, state.Invalid("vtb-stateless")};
  }

  auto id = vtb->getId();
  auto containingBlock_ptr = std::make_shared<VbkBlock>(vtb->containingBlock);

  // for the statefully invalid payloads we just save it for the future
  if (!mempool_tree_.acceptVTB(*vtb, containingBlock_ptr, state)) {
    vtbs_in_flight_[id] = vtb;
    on_vtb_accepted.emit(*vtb);
    return {FAILED_STATEFUL, state.Invalid("vtb-stateful")};
  }

  VBK_LOG_DEBUG("[POP mempool] VTB=%s is connected", id.toHex());
  auto& rel = getOrPutVbkRelation(containingBlock_ptr);
  rel.vtbs.push_back(vtb);
  makePayloadConnected<VTB>(vtb);

  return true;
}

template <>
MemPool::SubmitResult MemPool::submit<VbkBlock>(
    const std::shared_ptr<VbkBlock>& blk, ValidationState& state) {
  // stateless validation
  if (!checkBlock(
          *blk, state, mempool_tree_.vbk().getStableTree().getParams())) {
    return {FAILED_STATELESS, state.Invalid("vbkblock-stateless")};
  }

  auto id = blk->getId();

  // for the statefully invalid payloads we just save it for the future
  if (!mempool_tree_.acceptVbkBlock(blk, state)) {
    vbkblocks_in_flight_[id] = blk;
    on_vbkblock_accepted.emit(*blk);
    return {FAILED_STATEFUL, state.Invalid("vbk-stateful")};
  }

  // stateful validation
  VBK_LOG_DEBUG("[POP mempool] VbkBlock=%s is connected", id.toHex());
  if (mempool_tree_.vbk().getStableTree().getBlockIndex(blk->getHash()) ==
      nullptr) {
    getOrPutVbkRelation(blk);
  }

  vbkblocks_in_flight_.erase(blk->getId());

  return true;
}

void MemPool::tryConnectPayloads() {
  ValidationState state;

  // resubmit vbk blocks
  using P1 = std::pair<vbkblock_map_t::key_type, vbkblock_map_t::mapped_type>;
  std::vector<P1> blocks(vbkblocks_in_flight_.begin(),
                         vbkblocks_in_flight_.end());
  std::sort(blocks.begin(), blocks.end(), [](const P1& a, const P1& b) -> bool {
    return a.second->getHeight() < b.second->getHeight();
  });
  for (const auto& pair : blocks) {
    submit<VbkBlock>(pair.second, state);
  }

  // resubmit vtbs
  using P2 = std::pair<vtb_map_t::key_type, vtb_map_t::mapped_type>;
  std::vector<P2> vtbs(vtbs_in_flight_.begin(), vtbs_in_flight_.end());
  std::sort(vtbs.begin(), vtbs.end(), [](const P2& a, const P2& b) -> bool {
    return a.second->containingBlock.getHeight() <
           b.second->containingBlock.getHeight();
  });
  for (const auto& pair : vtbs) {
    submit<VTB>(pair.second, state);
  }

  // resubmit atvs
  using P3 = std::pair<atv_map_t::key_type, atv_map_t::mapped_type>;
  std::vector<P3> atvs(atvs_in_flight_.begin(), atvs_in_flight_.end());
  std::sort(atvs.begin(), atvs.end(), [](const P3& a, const P3& b) -> bool {
    return a.second->blockOfProof.getHeight() <
           b.second->blockOfProof.getHeight();
  });
  for (const auto& pair : atvs) {
    submit<ATV>(pair.second, state);
  }
}

template <>
const MemPool::vbkblock_map_t& MemPool::getMap() const {
  return vbkblocks_;
}

template <>
const MemPool::atv_map_t& MemPool::getMap() const {
  return stored_atvs_;
}

template <>
const MemPool::vtb_map_t& MemPool::getMap() const {
  return stored_vtbs_;
}

template <>
const MemPool::vbkblock_map_t& MemPool::getInFlightMap() const {
  return vbkblocks_in_flight_;
}

template <>
const MemPool::atv_map_t& MemPool::getInFlightMap() const {
  return atvs_in_flight_;
}

template <>
const MemPool::vtb_map_t& MemPool::getInFlightMap() const {
  return vtbs_in_flight_;
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
