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
  }

  // clear vtbs
  for (const auto& vtb : pop.vtbs) {
    stored_vtbs_.erase(vtb.getId());
  }

  // clear atvs
  for (const auto& atv : pop.atvs) {
    stored_atvs_.erase(atv.getId());
  }
}

MemPool::VbkPayloadsRelations& MemPool::touchVbkBlock(const VbkBlock& block) {
  auto sh = block.getId();
  vbkblocks_[sh] = std::make_shared<VbkBlock>(block);

  auto& val = relations_[sh];
  if (val == nullptr) {
    val = std::make_shared<VbkPayloadsRelations>(block);
  }

  return *val;
}

MempoolResult MemPool::submitAll(const PopData& pop) {
  MempoolResult r;
  for (const VbkBlock& block : pop.context) {
    ValidationState state;
    submit<VbkBlock>(block, state);
    r.context.emplace_back(block.getId(), state);
  }

  for (const VTB& vtb : pop.vtbs) {
    ValidationState state;
    submit<VTB>(vtb, state);
    r.vtbs.emplace_back(vtb.getId(), state);
  }

  for (const ATV& atv : pop.atvs) {
    ValidationState state;
    submit<ATV>(atv, state);
    r.atvs.emplace_back(atv.getId(), state);
  }

  return r;
}

template <>
bool MemPool::submit(const ATV& atv, ValidationState& state) {
  if (!checkATV(atv, state, *alt_chain_params_, *vbk_chain_params_)) {
    return state.Invalid("pop-mempool-submit-atv");
  }

  for (const auto& b : atv.context) {
    touchVbkBlock(b);
  }

  auto& rel = touchVbkBlock(atv.containingBlock);

  auto atvid = atv.getId();
  auto atvptr = std::make_shared<ATV>(atv);
  auto pair = std::make_pair(atvid, atvptr);
  rel.atvs.push_back(atvptr);

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
    touchVbkBlock(b);
  }

  auto& rel = touchVbkBlock(vtb.containingBlock);
  auto vtbid = vtb.getId();
  auto vtbptr = std::make_shared<VTB>(vtb);
  auto pair = std::make_pair(vtbid, vtbptr);
  rel.vtbs.push_back(vtbptr);

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

  touchVbkBlock(blk);

  return true;
}

}  // namespace altintegration
