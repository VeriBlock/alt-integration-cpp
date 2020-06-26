// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ADDVTB_HPP
#define ALTINTEGRATION_ADDVTB_HPP

#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/entities/vtb.hpp>

#include "veriblock/fmt.hpp"

namespace altintegration {

struct AddVTB : public Command {
  using block_t = VbkBlock;
  ~AddVTB() override = default;

  template <class VTB_t>
  AddVTB(AltTree& tree, VTB_t&& vtb)
      : tree_(tree), vtb_(std::forward<VTB_t>(vtb)) {}

  bool Execute(ValidationState& state) override {
    // add commands to the containing VBK block
    return tree_.vbk().addPayloads(
        vtb_.containingBlock.getHash(), {vtb_}, state);
  }

  void UnExecute() override {
    return tree_.vbk().unsafelyRemovePayload(vtb_.containingBlock,
                                             vtb_.getId());
  }

  size_t getId() const override { return vtb_.getId().getLow64(); }

  //! debug method. returns a string describing this command
  std::string toPrettyString(size_t level = 0) const override {
    return fmt::sprintf("%sAddVTB{id=%llu}", std::string(level, ' '), getId());
  };

 private:
  AltTree& tree_;
  VTB vtb_;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ADDVTB_HPP
