#include "veriblock/blockchain/alt_block_tree.hpp"

#include "veriblock/stateless_validation.hpp"

namespace altintegration {

AltTree::index_t* AltTree::getBlockIndex(
    const std::vector<uint8_t>& hash) const {
  auto it = block_index_.find(hash);
  return it == block_index_.end() ? nullptr : it->second.get();
}

AltTree::index_t* AltTree::touchBlockIndex(const hash_t& blockHash) {
  auto it = block_index_.find(blockHash);
  if (it != block_index_.end()) {
    return it->second.get();
  }

  auto newIndex = std::make_shared<index_t>();
  it = block_index_.insert({blockHash, std::move(newIndex)}).first;
  return it->second.get();
}

AltTree::index_t* AltTree::insertBlockHeader(const AltBlock& block) {
  auto hash = block.getHash();
  index_t* current = getBlockIndex(hash);
  if (current) {
    // it is a duplicate
    return current;
  }

  current = touchBlockIndex(hash);
  current->header = block;
  current->pprev = getBlockIndex(block.previousBlock);

  if (current->pprev) {
    // prev block found
    current->height = current->pprev->height + 1;
    current->chainWork = current->pprev->chainWork;
  } else {
    current->height = 0;
    current->chainWork = 0;
  }

  return current;
}

bool AltTree::bootstrap(ValidationState& state) {
  if (!block_index_.empty()) {
    return state.Error("already bootstrapped");
  }

  auto block = alt_config_->getBootstrapBlock();
  auto* index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  if (!block_index_.empty() && !getBlockIndex(block.getHash())) {
    return state.Error("block-index-no-genesis");
  }

  if (!cmp_.setState(*index, state)) {
    return state.Invalid("vbk-set-state");
  }

  return true;
}

bool AltTree::acceptBlock(const AltBlock& block, ValidationState& state) {
  // we must know previous block
  auto* prev = getBlockIndex(block.previousBlock);
  if (prev == nullptr) {
    return state.Invalid("bad-prev-block", "can not find previous block");
  }

  auto index = insertBlockHeader(block);

  assert(index != nullptr &&
         "insertBlockHeader should have never returned nullptr");

  (void)index;

  return true;
}

bool AltTree::addPayloads(const AltBlock& containingBlock,
                          const std::vector<payloads_t>& payloads,
                          ValidationState& state) {
  auto hash = containingBlock.getHash();
  auto* index = getBlockIndex(hash);
  if (!index) {
    return state.Invalid("no-alt-block",
                         "addPayloads can be executed only on existing "
                         "blocks, can not find block " +
                         HexStr(hash));
  }

  if (!cmp_.setState(*index, state)) {
    return state.Error("bad-state");
  }

  for (size_t i = 0, size = payloads.size(); i < size; i++) {
    auto& p = payloads[i];
    if (p.hasAtv && !checkPayloads(p.atv, state, vbk().getParams())) {
      return state.addIndex(i).Invalid("bad-atv-stateless");
    }
  }

  if (!cmp_.addPayloads(*index, payloads, state)) {
    return state.Invalid("bad-atv-stateful");
  }

  return true;
}

void AltTree::removePayloads(const AltBlock& containingBlock,
                             const std::vector<payloads_t>& payloads) {
  auto* index = getBlockIndex(containingBlock.getHash());
  assert(index);

  cmp_.removePayloads(*index, payloads);
}

template <>
bool AltTree::PopForkComparator::sm_t::applyContext(
    const BlockIndex<AltBlock>& index, ValidationState& state) {
  return tryValidateWithResources(
      [&]() -> bool {
        // apply vbk blocks after that
        auto& vbk = index.containingContext.vbk;
        for (size_t i = 0, size = vbk.size(); i < size; i++) {
          if (!tree().acceptBlock(vbk[i], state)) {
            return state.addIndex(i).Invalid("bad-vbk-block");
          }
        }

        return true;
      },
      [&] { unapplyContext(index); });
}

template <>
void AltTree::PopForkComparator::sm_t::unapplyContext(
    const BlockIndex<AltBlock>& index) {
  // unapply in "forward" order, because result should be same, but doing this
  // way it should be faster due to less number of calls "determineBestChain"

  for (const auto& b : index.containingContext.vbk) {
    tree().invalidateBlockByHash(b.getHash());
  }
}

template <>
void addContextToBlockIndex(BlockIndex<AltBlock>& index,
                            const typename BlockIndex<AltBlock>::payloads_t& p,
                            const VbkBlockTree& tree) {
  auto& ctx = index.containingContext.vbk;

  // only add blocks that are UNIQUE
  std::unordered_set<uint256> set;
  set.reserve(ctx.size());
  for (const auto& c : ctx) {
    set.insert(c.getHash());
  }

  auto add = [&](const VbkBlock& b) {
    auto hash = b.getHash();
    // filter context: add only blocks that are unknown and not in current 'ctx'
    if (!set.count(hash) && !tree.getBlockIndex(hash)) {
      ctx.push_back(b);
      set.insert(hash);
    }
  };

  // add context from every VTB
  for (const auto& vtb : p.vtbs) {
    for (const auto& b : vtb.context) {
      add(b);
    }
    add(vtb.containingBlock);
  }

  // add context from ATV
  if (p.hasAtv) {
    for (const auto& b : p.atv.context) {
      add(b);
    }
    add(p.atv.containingBlock);
  }
}

template <>
void removeContextFromBlockIndex(BlockIndex<AltBlock>& index,
                                 const BlockIndex<AltBlock>::payloads_t& p) {
  auto& ctx = index.containingContext.vbk;
  auto end = ctx.end();
  auto remove = [&](const VbkBlock& b) {
    end = std::remove(ctx.begin(), end, b);
  };

  // remove ATV context
  std::for_each(p.atv.context.rbegin(), p.atv.context.rbegin(), remove);
  // for every VTB, in reverse order
  std::for_each(p.vtbs.rbegin(), p.vtbs.rend(), [&](const VTB& vtb) {
    // remove context blocks, in reverse order
    std::for_each(vtb.context.rbegin(), vtb.context.rend(), remove);
  });

  ctx.erase(end, ctx.end());
}

// template <>
// void addContextToBlockIndex(
//    BlockIndex<AltBlock>& index,
//    const typename BlockIndex<AltBlock>::context_t& context,
//    const VbkBlockTree& tree) {
//  if (!index.containingContext.empty()) {
//    auto& ctx = index.containingContext.top();
//
//    // step 1
//    for (const auto& b : context.vbk) {
//      if (!tree.getBlockIndex(b.getHash())) {
//        ctx.vbk.push_back(b);
//      }
//    }
//    // step 2, process VTB info
//    for (const auto& vtb_info : context.vbkContext) {
//      auto* temp = tree.getBlockIndex(std::get<0>(vtb_info).getHash());
//
//      if (!temp || temp->containingEndorsements.find(
//                       std::get<1>(vtb_info).endorsement.id) ==
//                       temp->containingEndorsements.end()) {
//        ctx.vbkContext.push_back({std::get<0>(vtb_info),
//                                  std::get<1>(vtb_info),
//                                  {/* empty vector */}});
//
//        for (const auto& b : std::get<2>(vtb_info)) {
//          if (!tree.getBlockIndex(b.getHash())) {
//            std::get<2>(*ctx.vbkContext.rbegin()).push_back(b);
//          }
//        }
//      }
//    }
//  }
//}

}  // namespace altintegration
