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
    auto result =
        contextuallyCheckEndorsement<endorsement_t,
                                     ProtectedTree,
                                     ProtectingTree>(*e_, *ed_, *ing_, state);
    if (!result.valid) {
      return false;
    }

    result.containing->containingEndorsements.insert(
        std::make_pair(e_->id, e_));
    result.endorsed->endorsedBy.push_back(e_.get());

    return true;
  }

  void UnExecute() override {
    auto* containing = ed_->getBlockIndex(e_->containingHash);
    VBK_ASSERT(
        containing != nullptr &&
        "failed to roll back AddEndorsement: the containing block does not "
        "exist");

    auto* endorsed = containing->getAncestor(e_->endorsedHeight);
    VBK_ASSERT(
        endorsed != nullptr &&
        "failed to roll back AddEndorsement: the endorsed block does not "
        "exist");

    auto endorsement_it = containing->containingEndorsements.find(e_->id);
    VBK_ASSERT(endorsement_it != containing->containingEndorsements.end());

    // erase endorsedBy
    {
      auto& v = endorsed->endorsedBy;
      auto& id = e_->id;

      // find and erase the last occurrence of e_
      auto endorsed_it =
          std::find_if(v.rbegin(), v.rend(), [&id](endorsement_t* p) {
            return p->id == id;
          });

      VBK_ASSERT(
          endorsed_it != v.rend() &&
          "failed to roll back AddEndorsement: the endorsed block does not "
          "contain the endorsement in endorsedBy");

      auto toRemove = --(endorsed_it.base());
      v.erase(toRemove);
    }

    // erase endorsement
    containing->containingEndorsements.erase(endorsement_it);
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