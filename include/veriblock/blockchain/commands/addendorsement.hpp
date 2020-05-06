// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
      return state.Invalid("no-containing",
                           "Can not find containing block " + HexStr(index_));
    }

    // endorsement validity window
    auto window = ed_->getParams().getEndorsementSettlementInterval();
    auto minHeight = std::max(containing->height - window, 0);
    Chain<protected_index_t> chain(minHeight, containing);

    auto endorsedHeight = e_->endorsedHeight;
    if (containing->height - endorsedHeight > window) {
      return state.Invalid("expired", "Endorsement expired");
    }

    auto* endorsed = chain[endorsedHeight];
    if (!endorsed) {
      return state.Invalid("no-endorsed-block",
                           "No block found on endorsed block height");
    }

    if (endorsed->getHash() != e_->endorsedHash) {
      return state.Invalid("block-differs",
                           "Endorsed block is on a different chain");
    }

    auto* blockOfProof = ing_->getBlockIndex(e_->blockOfProof);
    if (!blockOfProof) {
      return state.Invalid("block-of-proof-not-found",
                           "Can not find block of proof in BTC");
    }

    auto* duplicate = chain.findBlockContainingEndorsement(*e_, window);
    if (duplicate) {
      // found duplicate
      return state.Invalid("duplicate",
                           "Found duplicate endorsement on the same chain");
    }

    containing->containingEndorsements[e_->id] = e_;
    endorsed->endorsedBy.push_back(e_.get());

    return true;
  }

  void UnExecute() override {
    auto* containing = ed_->getBlockIndex(e_->containingHash);
    if (!containing) {
      // can't find containing block
      return;
    }

    auto* endorsed = containing->getAncestor(e_->endorsedHeight);
    if (endorsed) {
      auto& v = endorsed->endorsedBy;
      // find last occurrence of e_
      auto it = std::find(v.rbegin(), v.rend(), e_.get());
      if (it != v.rend()) {
        // remove single item
        auto toRemove = --(it.base());
        v.erase(toRemove);
      }
    }
    containing->containingEndorsements.erase(e_->id);
  }

  size_t getId() const override { return e_->id.getLow64(); }

  std::string toPrettyString(size_t level = 0) const override {
    return std::string(level, ' ') + "Add" + e_->toPrettyString();
  }

 private:
  ProtectingTree* ing_;
  ProtectedTree* ed_;
  const hash_t index_;
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