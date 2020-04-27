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

template <typename E, typename ProtectingBlockTree, typename ProtectedTree>
struct AddEndorsement : public Command {
  using block_t = typename ProtectingBlockTree::block_t;
  using endorsement_t = E;
  using protected_param_t = typename ProtectedTree::params_t;
  using protected_index_t = typename ProtectedTree::index_t;

  ~AddEndorsement() override = default;

  explicit AddEndorsement(ProtectingBlockTree& tree,
                          const protected_param_t& param,
                          protected_index_t& index,
                          std::shared_ptr<endorsement_t> e)
      : tree_(&tree), param_(&param), index_(&index), e_(std::move(e)) {}

  bool Execute(ValidationState& state) override {
    // endorsement validity window
    auto window = param_->getEndorsementSettlementInterval();
    auto minHeight = std::max(index_->height - window, 0);
    Chain<protected_index_t> chain(minHeight, index_);

    auto endorsedHeight = e_->endorsedHeight;
    if (index_->height - endorsedHeight > window) {
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

    auto* blockOfProof = tree_->getBlockIndex(e_->blockOfProof);
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

    index_->containingEndorsements[e_->id] = e_;
    endorsed->endorsedBy.push_back(e_.get());

    return true;
  }

  void UnExecute() override {
    auto* endorsed = index_->getAncestor(e_->endorsedHeight);
    if (endorsed) {
      auto& v = endorsed->endorsedBy;
      // TODO: reverse iteration may be faster, as items "to be removed" are
      // likely exist at the end of vector
      v.erase(std::remove(v.begin(), v.end(), e_.get()), v.end());
    }
    index_->containingEndorsements.erase(e_->id);
  }

  std::string toPrettyString(size_t level = 0) const override {
    return std::string(level, ' ') + "Add" + e_->toPrettyString();
  }

 private:
  ProtectingBlockTree* tree_;
  const protected_param_t* param_;
  protected_index_t* index_;
  std::shared_ptr<endorsement_t> e_;
};

struct AltTree;
struct VbkBlockTree;
template <typename Block, typename ChainParams>
struct BlockTree;

using AddBtcEndorsement = AddEndorsement<BtcEndorsement,
                                         BlockTree<BtcBlock, BtcChainParams>,
                                         VbkBlockTree>;

using AddVbkEndorsement = AddEndorsement<VbkEndorsement, VbkBlockTree, AltTree>;

}  // namespace altintegration

#endif  // ALTINTEGRATION_ADDENDORSEMENT_HPP
