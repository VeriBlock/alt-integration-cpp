#include "veriblock/blockchain/blockchain_util.hpp"
#include "veriblock/blockchain/mempool_block_tree.hpp"
#include "veriblock/fmt.hpp"
#include "veriblock/keystone_util.hpp"

namespace altintegration {

bool MemPoolBlockTree::acceptVbkBlock(const std::shared_ptr<VbkBlock>& blk,
                                      ValidationState& state) {
  return temp_vbk_tree_.acceptBlock(blk, state);
}

bool MemPoolBlockTree::checkContextually(const VbkBlock& block,
                                         ValidationState& state) {
  auto& vbkstable = vbk().getStableTree();
  auto hash = block.getHash();
  auto index = vbkstable.getBlockIndex(hash);
  if (index != nullptr) {
    // duplicate
    return state.Invalid("duplicate");
  }

  auto* tip = vbkstable.getBestChain().tip();
  VBK_ASSERT(tip);
  bool tooOld = tip->getHeight() - vbkstable.getParams().getMaxReorgBlocks() >
                block.getHeight();
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
        fmt::sprintf("ATV=%s already added to active chain in block %s",
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
                           fmt::sprintf("ATV=%s expired %s",
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
        fmt::sprintf("VTB=%s already added to active chain in block %s",
                     vtb.getId().toHex(),
                     duplicate->toShortPrettyString()));
  }

  if (vtb.containingBlock.getHeight() >
      window + vtb.transaction.publishedBlock.getHeight()) {
    return state.Invalid(
        "vtb-expired",
        fmt::sprintf("VTB=%s expired %s",
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

  if (!this->temp_vbk_tree_.acceptBlock(containingBlock, state)) {
    return state.Invalid("bad-containing-block");
  }

  size_t i = 0;
  for (const auto& blk : vtb.transaction.blockOfProofContext) {
    if (!temp_btc_tree_.acceptBlock(blk, state)) {
      return state.Invalid("bad-block-of-proof-context", i);
    }

    i++;
  }

  if (!temp_btc_tree_.acceptBlock(vtb.transaction.blockOfProof, state)) {
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

  if (!temp_vbk_tree_.acceptBlock(blockOfProof, state)) {
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

}  // namespace altintegration