// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ADDENDORSEMENT_HPP
#define ALTINTEGRATION_ADDENDORSEMENT_HPP

#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/blockchain/command.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/endorsements.hpp>

#include "veriblock/fmt.hpp"

namespace altintegration {

//! @private
template <typename ProtectingTree, typename ProtectedTree>
struct AddEndorsement : public Command {
  using hash_t = typename ProtectedTree::hash_t;
  using protected_block_t = typename ProtectedTree::block_t;
  using protecting_block_t = typename ProtectingTree::block_t;
  using endorsement_t = typename ProtectedTree::index_t::endorsement_t;
  using protected_index_t = typename ProtectedTree::index_t;

  ~AddEndorsement() override = default;

  explicit AddEndorsement(ProtectingTree& ing,
                          ProtectedTree& ed,
                          std::shared_ptr<endorsement_t> e)
      : ing_(&ing), ed_(&ed), e_(std::move(e)) {}

  bool Execute(ValidationState& state) override {
    auto* containing = ed_->getBlockIndex(e_->containingHash);
    if (!containing) {
      return state.Invalid(
          protected_block_t::name() + "-no-containing",
          fmt::sprintf("Can not find containing block in endorsement=%s",
                       e_->toPrettyString()));
    }

    // endorsement validity window
    auto window = ed_->getParams().getEndorsementSettlementInterval();
    auto minHeight = (std::max)(containing->getHeight() - window, 0);
    Chain<protected_index_t> chain(minHeight, containing);

    auto* endorsed = ed_->getBlockIndex(e_->endorsedHash);
    if (!endorsed) {
      return state.Invalid(protected_block_t::name() + "-no-endorsed-block",
                           "Endorsed block not found in the tree");
    }

    if (containing->getHeight() - endorsed->getHeight() > window) {
      return state.Invalid(protected_block_t::name() + "-expired",
                           "Endorsement expired");
    }

    if (chain[endorsed->getHeight()] == nullptr ||
        endorsed->getHash() != chain[endorsed->getHeight()]->getHash()) {
      return state.Invalid(
          protected_block_t::name() + "-block-differs",
          fmt::sprintf(
              "Endorsed block is on a different chain. Expected: %s, got %s",
              endorsed->toShortPrettyString(),
              HexStr(e_->endorsedHash)));
    }

    auto* blockOfProof = ing_->getBlockIndex(e_->blockOfProof);
    if (!blockOfProof) {
      return state.Invalid(
          protected_block_t::name() + "-block-of-proof-not-found",
          fmt::sprintf("Can not find block of proof in SP Chain (%s)",
                       HexStr(e_->blockOfProof)));
    }

    containing->insertContainingEndorsement(e_);
    endorsed->endorsedBy.push_back(e_.get());
    blockOfProof->blockOfProofEndorsements.push_back(e_.get());

    return true;
  }

  void UnExecute() override {
    auto* containing = ed_->getBlockIndex(e_->containingHash);
    VBK_ASSERT_MSG(
        containing != nullptr,
        "failed to roll back AddEndorsement: the containing block does not "
        "exist %s",
        e_->toPrettyString());

    auto* endorsed = ed_->getBlockIndex(e_->endorsedHash);
    VBK_ASSERT_MSG(
        endorsed != nullptr,
        "failed to roll back AddEndorsement: the endorsed block does not "
        "exist %s",
        e_->toPrettyString());

    auto* blockOfProof = ing_->getBlockIndex(e_->blockOfProof);
    VBK_ASSERT_MSG(blockOfProof != nullptr,
                   "failed to roll back AddEndorsement: the blockOfProof "
                   "block does not exist %s",
                   e_->toPrettyString());

    // e_ is likely to have different address than one stored in a Block.
    // make sure that we use correct Endorsement instance, then to remove proper
    // ptrs from endorsedBy and blockOfProofEndorsements
    auto Eit = containing->findContainingEndorsement(e_->id);
    VBK_ASSERT_MSG(Eit != containing->getContainingEndorsements().end(),
                   "state corruption: containing endorsement not found");

    // we added endorsements by ptr, so find them by ptr
    auto rm = [&](const endorsement_t* e) -> bool {
      return e == (Eit->second).get();
    };

    // erase endorsedBy
    bool p1 = erase_last_item_if<endorsement_t>(endorsed->endorsedBy, rm);
    VBK_ASSERT_MSG(p1,
                   "Failed to remove endorsement %s from endorsedBy in "
                   "AddEndorsement::Unexecute",
                   e_->toPrettyString());

    // erase blockOfProof
    bool p2 = erase_last_item_if<endorsement_t>(
        blockOfProof->blockOfProofEndorsements, rm);
    VBK_ASSERT_MSG(p2,
                   "Failed to remove endorsement %s from blockOfProof in "
                   "AddEndorsement::Unexecute",
                   e_->toPrettyString());

    // erase containing, should be removed last
    containing->removeContainingEndorsement(Eit);
  }

  size_t getId() const override { return e_->id.getLow64(); }

  std::string toPrettyString(size_t level = 0) const override {
    return fmt::sprintf(
        "%sAdd%s", std::string(level, ' '), e_->toPrettyString());
  }

 private:
  ProtectingTree* ing_;
  ProtectedTree* ed_;
  std::shared_ptr<endorsement_t> e_;
};

struct AltTree;
struct VbkBlockTree;
template <typename Block, typename ChainParams>
struct BlockTree;

using AddVbkEndorsement =
    AddEndorsement<BlockTree<BtcBlock, BtcChainParams>, VbkBlockTree>;

using AddAltEndorsement = AddEndorsement<VbkBlockTree, AltTree>;

}  // namespace altintegration

#endif  // ALTINTEGRATION_ADDENDORSEMENT_HPP
