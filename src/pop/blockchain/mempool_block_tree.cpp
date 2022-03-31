#include "veriblock/pop/blockchain/mempool_block_tree.hpp"

#include "veriblock/pop/blockchain/alt_block_tree_util.hpp"
#include "veriblock/pop/blockchain/blockchain_util.hpp"
#include "veriblock/pop/blockchain/pop/counting_context.hpp"
#include "veriblock/pop/fmt.hpp"
#include "veriblock/pop/keystone_util.hpp"
#include "veriblock/pop/validation_state.hpp"

namespace altintegration {

namespace {

static void assertPopDataFits(PopData& pop, const AltChainParams& params) {
  const auto& maxSize = params.getMaxPopDataSize();
  const auto& maxVbkBlocks = params.getMaxVbkBlocksInAltBlock();
  const auto& maxVTBs = params.getMaxVTBsInAltBlock();
  const auto& maxATVs = params.getMaxATVsInAltBlock();
  VBK_ASSERT(pop.context.size() <= maxVbkBlocks);
  VBK_ASSERT(pop.vtbs.size() <= maxVTBs);
  VBK_ASSERT(pop.atvs.size() <= maxATVs);
  const auto estimate = pop.estimateSize();
  VBK_ASSERT_MSG(estimate <= maxSize, "estimate=%d, max=%d", estimate, maxSize);
}

}  // namespace

bool MemPoolBlockTree::acceptVbkBlock(const std::shared_ptr<VbkBlock>& blk,
                                      ValidationState& state) {
  return temp_vbk_tree_.acceptBlockHeader(blk, state);
}

bool MemPoolBlockTree::checkContextually(const VbkBlock& block,
                                         ValidationState& state) {
  auto& vbkstable = vbk().getStableTree();
  const auto& hash = block.getHash();
  auto index = vbkstable.getBlockIndex(hash);
  if (index != nullptr) {
    // duplicate
    return state.Invalid("duplicate");
  }

  auto* tip = vbkstable.getBestChain().tip();
  VBK_ASSERT(tip);
  bool tooOld = tip->getHeight() - block.getHeight() >
                vbkstable.getParams().getMaxReorgBlocks();
  if (tooOld) {
    return state.Invalid("too-old");
  }

  return true;
}

bool MemPoolBlockTree::checkContextually(const ATV& atv,
                                         ValidationState& state) {
  // stateful validation
  int32_t window = tree_->getParams().getEndorsementSettlementInterval();
  auto duplicate = findBlockContainingEndorsement(
      tree_->getBestChain(), tree_->getBestChain().tip(), atv.getId(), window);
  if (duplicate != nullptr) {
    return state.Invalid(
        "atv-duplicate",
        format("ATV={} already added to active chain in block {}",
               atv.getId().toHex(),
               duplicate->toShortPrettyString()));
  }

  auto endorsed_hash =
      tree_->getParams().getHash(atv.transaction.publicationData.header);

  auto* endorsed_index = tree_->getBlockIndex(endorsed_hash);
  if (endorsed_index != nullptr) {
    auto* tip = tree_->getBestChain().tip();
    VBK_ASSERT_MSG(tip != nullptr, "block tree is not bootstrapped");

    if (tip->getHeight() + 1 > window + endorsed_index->getHeight()) {
      return state.Invalid("atv-expired",
                           format("ATV={} expired {}",
                                  atv.getId().toHex(),
                                  endorsed_index->toShortPrettyString()));
    }
  }

  return true;
}

bool MemPoolBlockTree::checkContextually(const VTB& vtb,
                                         ValidationState& state) {
  auto& vbk = temp_vbk_tree_.getStableTree();
  auto* containing = vbk.getBlockIndex(vtb.containingBlock.getHash());
  int32_t window = vbk.getParams().getEndorsementSettlementInterval();
  auto duplicate = findBlockContainingEndorsement(
      vbk.getBestChain(),
      // if containing exists on chain, then search for duplicates starting from
      // containing, else search starting from tip
      (containing != nullptr ? containing : vbk.getBestChain().tip()),
      vtb.getId(),
      window);
  if (duplicate != nullptr) {
    return state.Invalid(
        "vtb-duplicate",
        format("VTB={} already added to active chain in block {}",
               vtb.getId().toHex(),
               duplicate->toShortPrettyString()));
  }

  if (vtb.containingBlock.getHeight() >
      window + vtb.transaction.publishedBlock.getHeight()) {
    return state.Invalid(
        "vtb-expired",
        format("VTB={} expired {}",
               vtb.getId().toHex(),
               vtb.transaction.publishedBlock.toPrettyString()));
  }

  return true;
}

bool MemPoolBlockTree::acceptVTB(
    const VTB& vtb,
    const std::shared_ptr<VbkBlock>& containingBlock,
    ValidationState& state) {
  VBK_ASSERT_MSG(
      containingBlock->getHash() == vtb.containingBlock.getHash(),
      "containingBlock should be equal to the vtb containingBlock block");

  if (!checkContextually(vtb, state)) {
    return state.Invalid("vtb-contextual");
  }

  if (!this->temp_vbk_tree_.acceptBlockHeader(containingBlock, state)) {
    return state.Invalid("bad-containing-block");
  }

  size_t i = 0;
  for (const auto& blk : vtb.transaction.blockOfProofContext) {
    if (!temp_btc_tree_.acceptBlockHeader(blk, state)) {
      return state.Invalid("bad-block-of-proof-context", i);
    }

    i++;
  }

  if (!temp_btc_tree_.acceptBlockHeader(vtb.transaction.blockOfProof, state)) {
    return state.Invalid("bad-block-of-proof");
  }

  return true;
}

bool MemPoolBlockTree::acceptATV(const ATV& atv,
                                 const std::shared_ptr<VbkBlock>& blockOfProof,
                                 ValidationState& state) {
  VBK_ASSERT_MSG(
      blockOfProof->getHash() == atv.blockOfProof.getHash(),
      "containingBlock should be equal to the atv blockOfProof block");

  if (!checkContextually(atv, state)) {
    return state.Invalid("atv-contextual");
  }

  if (!temp_vbk_tree_.acceptBlockHeader(blockOfProof, state)) {
    return state.Invalid("bad-block-of-proof");
  }

  return true;
}

bool MemPoolBlockTree::areStronglyEquivalent(const ATV& atv1, const ATV& atv2) {
  return atv1.transaction.getHash() == atv2.transaction.getHash() &&
         atv1.blockOfProof.getHash() == atv2.blockOfProof.getHash();
}

bool MemPoolBlockTree::areStronglyEquivalent(const VTB& vtb1, const VTB& vtb2) {
  return (vtb1.transaction.bitcoinTransaction ==
          vtb2.transaction.bitcoinTransaction) &&
         (vtb1.transaction.blockOfProof == vtb2.transaction.blockOfProof);
}

bool MemPoolBlockTree::areWeaklyEquivalent(const VTB& vtb1, const VTB& vtb2) {
  bool is_on_the_same_btc_chain = areOnSameChain(vtb1.transaction.blockOfProof,
                                                 vtb2.transaction.blockOfProof,
                                                 tree_->btc());

  bool is_on_the_same_vbk_chain =
      areOnSameChain(vtb1.transaction.publishedBlock,
                     vtb2.transaction.publishedBlock,
                     tree_->vbk());

  bool are_the_same_keystone_period =
      areOnSameKeystoneInterval(vtb1.transaction.publishedBlock.getHeight(),
                                vtb2.transaction.publishedBlock.getHeight(),
                                tree_->vbk().getParams().getKeystoneInterval());

  return ((vtb1.transaction.publishedBlock ==
           vtb2.transaction.publishedBlock) &&
          is_on_the_same_btc_chain) ||
         (is_on_the_same_vbk_chain && are_the_same_keystone_period &&
          is_on_the_same_btc_chain);
}

int MemPoolBlockTree::weaklyCompare(const VTB& vtb1, const VTB& vtb2) {
  VBK_ASSERT_MSG(areWeaklyEquivalent(vtb1, vtb2),
                 "vtbs should be weakly equivalent");

  auto* blockOfProof1 =
      tree_->btc().getBlockIndex(vtb1.transaction.blockOfProof.getHash());
  auto* blockOfProof2 =
      tree_->btc().getBlockIndex(vtb2.transaction.blockOfProof.getHash());

  VBK_ASSERT_MSG(blockOfProof1,
                 "unknown block %s",
                 vtb1.transaction.blockOfProof.toPrettyString());
  VBK_ASSERT_MSG(blockOfProof2,
                 "unknown block %s",
                 vtb2.transaction.blockOfProof.toPrettyString());

  if (blockOfProof1->getHeight() > blockOfProof2->getHeight()) {
    return -1;
  }

  if (blockOfProof1->getHeight() < blockOfProof2->getHeight()) {
    return 1;
  }

  if (vtb1.transaction.publishedBlock.getHeight() >
      vtb2.transaction.publishedBlock.getHeight()) {
    return -1;
  }

  if (vtb1.transaction.publishedBlock.getHeight() <
      vtb2.transaction.publishedBlock.getHeight()) {
    return 1;
  }

  if (vtb1.transaction.blockOfProofContext.size() >
      vtb2.transaction.blockOfProofContext.size()) {
    return 1;
  }

  if (vtb1.transaction.blockOfProofContext.size() <
      vtb2.transaction.blockOfProofContext.size()) {
    return -1;
  }

  VBK_ASSERT_MSG(
      areStronglyEquivalent(vtb1, vtb2),
      "if we can not define the best vtb they should be strongly equivalent");
  return 0;
}

bool MemPoolBlockTree::isBlockOld(const VbkBlock& block) const {
  return tree_->vbk().isBlockOld(block.getHeight());
}

template <typename Payload>
void applyPayloadsOrRemoveIfInvalid(
    AltBlockTree::BlockPayloadMutator& mutator,
    std::vector<Payload>& payloads,
    CountingContext& context,
    const std::function<void(const Payload&, const ValidationState&)>&
        onPayload) {
  auto it = std::remove_if(
      payloads.begin(), payloads.end(), [&](const Payload& payload) {
        ValidationState state;

        // will be executed when function exits
        auto finalizer = Finalizer([&]() { onPayload(payload, state); });

        if (!context.canFit(payload)) {
          // can't fit, doesn't worth even trying to add this payload
          state.Invalid("does-not-fit", "Can't fit into a block");
          return true;  // should be removed
        }

        if (mutator.isStatelessDuplicate(getIdVector(payload))) {
          state.Invalid("stateless-duplicate",
                        format("Stateless duplicate payload with id {}",
                               HexStr(payload.getId())));
          return true;  // should be removed
        }

        if (!mutator.add(payload, state)) {
          // payload is invalid and should be removed
          return true;
        }

        // actually update context ONLY if payload is valid and applied
        context.update(payload);
        return false;  // do not remove this payload
      });
  payloads.erase(it, payloads.end());
}

void MemPoolBlockTree::filterInvalidPayloads(
    PopData& pop,
    const std::function<void(const ATV&, const ValidationState&)>& onATV,
    const std::function<void(const VTB&, const ValidationState&)>& onVTB,
    const std::function<void(const VbkBlock&, const ValidationState&)>& onVBK) {
  // return early
  if (pop.empty()) {
    return;
  }

  VBK_LOG_WARN("Trying to add %s to next block...", pop.toPrettyString());

  // suppress the VBK fork resolution as we don't care about the best chain
  auto guard = tree_->vbk().deferForkResolutionGuard();
  auto originalTip = tree_->vbk().getBestChain().tip();

  // first, create tmp alt block
  AltBlock tmp;
  ValidationState state;
  {
    auto& tip = *tree_->getBestChain().tip();
    tmp.hash = std::vector<uint8_t>(32, 2);
    tmp.previousBlock = tip.getHash();
    tmp.timestamp = tip.getTimestamp() + 1;
    tmp.height = tip.getHeight() + 1;
    bool ret = tree_->acceptBlockHeader(tmp, state);
    VBK_ASSERT(ret);
  }

  auto* tmpindex = tree_->getBlockIndex(tmp.getHash());
  VBK_ASSERT(tmpindex != nullptr);

  tree_->getPayloadsProvider().writePayloads(pop);

  // create and activate the mempool block
  tree_->acceptBlock(*tmpindex, {});
  ValidationState dummy;
  bool success = tree_->setState(*tmpindex, dummy);
  VBK_ASSERT(success);

  CountingContext counter(tree_->getParams());
  auto mutator = tree_->makeConnectedLeafPayloadMutator(*tmpindex);

  applyPayloadsOrRemoveIfInvalid(mutator, pop.context, counter, onVBK);
  applyPayloadsOrRemoveIfInvalid(mutator, pop.vtbs, counter, onVTB);
  applyPayloadsOrRemoveIfInvalid(mutator, pop.atvs, counter, onATV);

  // assert PopData does not surpass limits
  assertPopDataFits(pop, tree_->getParams());

  VBK_LOG_WARN("Filtered valid: %s", pop.toPrettyString());

  // at this point `pop` contains only valid payloads
  tree_->removeSubtree(*tmpindex);

  tmpindex->disconnectFromPrev();
  tree_->deallocateBlock(*tmpindex);

  guard.overrideDeferredForkResolution(originalTip);
}
}  // namespace altintegration
