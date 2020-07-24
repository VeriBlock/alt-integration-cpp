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

template <>
void MemPool::remove<VbkBlock>(const VbkBlock::id_t& id) {
  vbkblocks_.erase(id);

  auto it = relations_.find(id);
  if (it == relations_.end()) {
    return;
  }

  for (auto& vtb : it->second->vtbs) {
    stored_vtbs_.erase(vtb->getId());
  }

  for (auto& atv : it->second->atvs) {
    stored_atvs_.erase(atv->getId());
  }

  relations_.erase(it);
}

template <>
void MemPool::remove<VTB>(const VTB::id_t& id) {
  auto it = stored_vtbs_.find(id);
  if (it == stored_vtbs_.end()) {
    // nothing to remove
    return;
  }

  // remove VTB from relation
  auto containingId = it->second->containingBlock.getId();
  auto rit = relations_.find(containingId);
  if (rit == relations_.end()) {
    return;
  }

  rit->second->removeVTB(id);
  stored_vtbs_.erase(it);
}

template <>
void MemPool::remove<ATV>(const ATV::id_t& id) {
  auto it = stored_atvs_.find(id);
  if (it == stored_atvs_.end()) {
    // nothing to remove
    return;
  }

  // remove ATV from relation
  auto containingId = it->second->blockOfProof.getId();
  auto rit = relations_.find(containingId);
  if (rit == relations_.end()) {
    return;
  }

  rit->second->removeATV(id);
  stored_atvs_.erase(it);
}

void MemPool::vacuum(const PopData& pop, const AltTree& tree) {
  // save ids of all Vbk Blocks that have been added into last block
  std::set<VbkBlock::id_t> blocks;
  std::transform(pop.context.begin(),
                 pop.context.end(),
                 std::inserter(blocks, std::end(blocks)),
                 get_id<VbkBlock>);

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
    auto* index = tree.vbk().getBlockIndex(blockHash);
    auto* tip = tree.vbk().getBestChain().tip();
    auto maxReorgBlocks = tree.vbk().getParams().getMaxReorgBlocks();
    auto& rel = *it->second;

    bool tooOld = tip->getHeight() - maxReorgBlocks > rel.header->height;
    if (tooOld) {
      // VBK block is too old to be included or modified
      it = removeRelation(it);
      continue;
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

      for (auto& vtb : rel.vtbs) {
        auto id = vtb->getId();
        if (ids.count(id)) {
          // mempool contains VBK block, which already exists in VBK tree, and
          // we found a VTB which exists in that VBK block. we can remove VTB
          // from mempool
          remove<VTB>(id);
        }
      }

      // does this VBK block contain any new VTB/ATV payloads?
      if (rel.empty()) {
        // it does not... we can remove this VBK block
        it = removeRelation(it);
        continue;
      }

      // this relation still has something useful
    }

    if (blocks.count(it->first)) {
      // this VBK block is added to new ALT block, and it has no payloads
      if (rel.empty()) {
        it = removeRelation(it);
        continue;
      }
    }

    ++it;
  }
}

void MemPool::removePayloads(const PopData& pop, const AltTree& tree) {
  for (auto& vtb : pop.vtbs) {
    remove<VTB>(vtb.getId());
  }
  for (auto& atv : pop.atvs) {
    remove<ATV>(atv.getId());
  }

  // vbk blocks will be cleaned during vacuum
  vacuum(pop, tree);
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

  on_vbkblock_accepted.emit(block);

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

  on_atv_accepted.emit(atv);

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

  on_vtb_accepted.emit(vtb);

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
