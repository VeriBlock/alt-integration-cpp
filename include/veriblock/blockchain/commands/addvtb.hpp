// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ADDVTB_HPP
#define ALTINTEGRATION_ADDVTB_HPP

#include <vector>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/entities/altblock.hpp>

namespace altintegration {

struct AddVTB : public Command {
  ~AddVTB() override = default;

  AddVTB(AltTree& tree, const VTB& vtb) : tree_(&tree), vtb_(vtb) {
    id_ = vtb.getId().getLow64();
  }

  bool Execute(ValidationState& state) override {
    // add commands to VBK containing block
    return tree_->vbk().addPayloads(
        vtb_.containingBlock.getHash(), {vtb_}, state);
  }
  void UnExecute() override {
    auto hash = vtb_.containingBlock.getHash();
    auto* index = tree_->vbk().getBlockIndex(hash);
    if (!index) {
      // this block no longer exists, do not remove anything
      return;
    }

    tree_->vbk().removePayloads(vtb_.containingBlock, {vtb_});
  }

  size_t getId() const override { return id_; }

  //! debug method. returns a string describing this command
  std::string toPrettyString(size_t level = 0) const override {
    std::ostringstream ss;
    std::string pad(level, ' ');
    ss << pad << "AddVTB";
    return ss.str();
  };

 private:
  AltTree* tree_;
  VTB vtb_;
  size_t id_;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ADDVTB_HPP
