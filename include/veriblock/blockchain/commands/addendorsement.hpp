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

template <typename ProtectingTree, typename ProtectedTree>
struct AddEndorsement : public Command {
  using hash_t = typename ProtectedTree::hash_t;
  using block_t = typename ProtectingTree::block_t;
  using endorsement_t = typename ProtectedTree::block_t::endorsement_t;
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
          block_t::name() + "-no-containing",
          fmt::sprintf("Can not find containing block in endorsement=%s",
                       e_->toPrettyString()));
    }

    // endorsement validity window
    auto window = ed_->getParams().getEndorsementSettlementInterval();
    auto minHeight = (std::max)(containing->height - window, 0);
    Chain<protected_index_t> chain(minHeight, containing);

    auto endorsedHeight = e_->endorsedHeight;
    if (containing->height - endorsedHeight > window) {
      return state.Invalid(block_t::name() + "-expired", "Endorsement expired");
    }

    auto* endorsed = chain[endorsedHeight];
    if (!endorsed) {
      return state.Invalid(block_t::name() + "-no-endorsed-block",
                           "No block found on endorsed block height");
    }

    if (endorsed->getHash() != e_->endorsedHash) {
      return state.Invalid(
          block_t::name() + "-block-differs",
          fmt::sprintf(
              "Endorsed block is on a different chain. Expected: %s, got %s",
              endorsed->toPrettyString(),
              HexStr(e_->endorsedHash)));
    }

    auto* blockOfProof = ing_->getBlockIndex(e_->blockOfProof);
    if (!blockOfProof) {
      return state.Invalid(
          block_t::name() + "-block-of-proof-not-found",
          fmt::sprintf("Can not find block of proof in SP Chain (%s)",
                       HexStr(e_->blockOfProof)));
    }

    if (endorsement_t::checkForDuplicates()) {
      auto* duplicate = chain.findBlockContainingEndorsement(*e_, window);
      if (duplicate) {
        // found duplicate
        return state.Invalid(
            block_t ::name() + "-duplicate",
            fmt::sprintf("Can not add endorsement=%s to block=%s, because we "
                         "found its duplicate in block %s",
                         e_->toPrettyString(),
                         containing->toPrettyString(),
                         duplicate->toPrettyString()));
      }
    }

    containing->containingEndorsements.insert(std::make_pair(e_->id, e_));
    endorsed->endorsedBy.push_back(e_.get());

    return true;
  }

  void UnExecute() override {
    auto* containing = ed_->getBlockIndex(e_->containingHash);
    assert(containing != nullptr &&
           "failed to roll back AddEndorsement: the containing block does not "
           "exist");

    auto* endorsed = containing->getAncestor(e_->endorsedHeight);

    assert(endorsed != nullptr &&
           "failed to roll back AddEndorsement: the endorsed block does not "
           "exist");

    {
      auto& v = endorsed->endorsedBy;

      // find and erase the last occurrence of e_
      auto endorsed_it = std::find(v.rbegin(), v.rend(), e_.get());

      assert(endorsed_it != v.rend() &&
             "failed to roll back AddEndorsement: the endorsed block does not "
             "contain the endorsement in endorsedBy");

      auto toRemove = --(endorsed_it.base());
      v.erase(toRemove);
    }

    {
      auto containing_it = containing->containingEndorsements.find(e_->id);
      assert(containing_it != containing->containingEndorsements.end() &&
             "failed to roll back AddEndorsement: the containing block does "
             "not contain the endorsement in containingEndorsements");

      containing->containingEndorsements.erase(containing_it);
    }
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

using AddBtcEndorsement =
    AddEndorsement<BlockTree<BtcBlock, BtcChainParams>, VbkBlockTree>;

using AddVbkEndorsement = AddEndorsement<VbkBlockTree, AltTree>;

}  // namespace altintegration

#endif  // ALTINTEGRATION_ADDENDORSEMENT_HPP
