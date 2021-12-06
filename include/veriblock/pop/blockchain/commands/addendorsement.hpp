// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ADDENDORSEMENT_HPP
#define ALTINTEGRATION_ADDENDORSEMENT_HPP

#include <veriblock/pop/blockchain/alt_chain_params.hpp>
#include <veriblock/pop/blockchain/block_index.hpp>
#include <veriblock/pop/blockchain/btc_chain_params.hpp>
#include <veriblock/pop/blockchain/chain.hpp>
#include <veriblock/pop/blockchain/command.hpp>
#include <veriblock/pop/blockchain/vbk_chain_params.hpp>
#include <veriblock/pop/entities/altblock.hpp>
#include <veriblock/pop/entities/endorsements.hpp>
#include <veriblock/pop/fmt.hpp>

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

  bool Execute(ValidationState& state) noexcept override {
    auto* containing = ed_->getBlockIndex(e_->containingHash);
    if (!containing) {
      return state.Invalid(
          protected_block_t::name() + "-no-containing",
          format("Can not find containing block in endorsement={}",
                 e_->toPrettyString()));
    }

    auto* endorsed = ed_->getBlockIndex(e_->endorsedHash);
    if (!endorsed) {
      return state.Invalid(protected_block_t::name() + "-no-endorsed-block",
                           format("Endorsed block={} not found in the tree",
                                  HexStr(e_->endorsedHash)));
    }

    auto actualEndorsed = containing->getAncestor(endorsed->getHeight());
    if (actualEndorsed == nullptr || endorsed != actualEndorsed) {
      return state.Invalid(
          protected_block_t::name() + "-block-differs",
          format("Endorsed block is on a different chain. Expected: {}, got {}",
                 endorsed->toShortPrettyString(),
                 (actualEndorsed ? actualEndorsed->toShortPrettyString()
                                 : "nullptr")));
    }

    if (containing->getHeight() - endorsed->getHeight() >
        (int)ed_->getParams().getEndorsementSettlementInterval()) {
      return state.Invalid(protected_block_t::name() + "-expired",
                           "Endorsement expired");
    }

    auto* blockOfProof = ing_->getBlockIndex(e_->blockOfProof);
    if (!blockOfProof) {
      return state.Invalid(
          protected_block_t::name() + "-block-of-proof-not-found",
          format("Can not find block of proof in SP Chain ({})",
                 HexStr(e_->blockOfProof)));
    }

    containing->insertContainingEndorsement(e_);
    endorsed->insertEndorsedBy(e_.get());
    blockOfProof->insertBlockOfProofEndorsement(e_.get());

    return true;
  }

  void UnExecute() noexcept override {
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

    // erase endorsedBy
    bool p1 = endorsed->eraseLastFromEndorsedBy(Eit->second.get());
    VBK_ASSERT_MSG(p1,
                   "Failed to remove endorsement %s from endorsedBy in "
                   "AddEndorsement::Unexecute",
                   e_->toPrettyString());

    // erase blockOfProof
    bool p2 =
        blockOfProof->eraseLastFromBlockOfProofEndorsement(Eit->second.get());
    VBK_ASSERT_MSG(p2,
                   "Failed to remove endorsement %s from blockOfProof in "
                   "AddEndorsement::Unexecute",
                   e_->toPrettyString());

    // erase containing, should be removed last
    containing->removeContainingEndorsement(Eit);
  }

 private:
  ProtectingTree* ing_ = nullptr;
  ProtectedTree* ed_ = nullptr;
  std::shared_ptr<endorsement_t> e_ = nullptr;
};

struct AltBlockTree;
struct VbkBlockTree;
template <typename Block, typename ChainParams>
struct BlockTree;

//! @private
using AddVbkEndorsement =
    AddEndorsement<BlockTree<BtcBlock, BtcChainParams>, VbkBlockTree>;

//! @private
using AddAltEndorsement = AddEndorsement<VbkBlockTree, AltBlockTree>;

}  // namespace altintegration

#endif  // ALTINTEGRATION_ADDENDORSEMENT_HPP
