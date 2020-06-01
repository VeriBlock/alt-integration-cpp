// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ADDVTB_HPP
#define ALTINTEGRATION_ADDVTB_HPP

#include <vector>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/entities/altblock.hpp>

#include "veriblock/fmt.hpp"

namespace altintegration {

struct AddVTB : public Command {
  ~AddVTB() override = default;

  AddVTB(AltTree& tree, const VTB& vtb) : tree_(&tree), vtb_(vtb) {
    id_ = vtb.getId().getLow64();
  }

  bool Execute(ValidationState& state) override {
    // add commands to VBK containing block
    auto containing = vtb_.containingBlock.getHash();
    auto& vbk = tree_->vbk();

    // addPayloads changes state. if we can't add payloads, immediately clear
    // all side effects
    if (!vbk.addPayloads(containing, {vtb_}, state)) {
      vbk.removePayloads(containing, {vtb_});
      return false;
    }

    return true;
  }
  void UnExecute() override {
    auto hash = vtb_.containingBlock.getHash();

    auto* index = tree_->vbk().getBlockIndex(hash);
    VBK_ASSERT(index != nullptr &&
           "failed to roll back addVTB: the containing block does not exist");

    tree_->vbk().removePayloads(vtb_.containingBlock, {vtb_});
  }

  size_t getId() const override { return id_; }

  //! debug method. returns a string describing this command
  std::string toPrettyString(size_t level = 0) const override {
    return fmt::sprintf("%sAddVTB{id=%llu}", std::string(level, ' '), id_);
  };

 private:
  AltTree* tree_;
  VTB vtb_;
  size_t id_;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ADDVTB_HPP
