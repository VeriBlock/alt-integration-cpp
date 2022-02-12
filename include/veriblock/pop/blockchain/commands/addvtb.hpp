// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ADDVTB_HPP
#define ALTINTEGRATION_ADDVTB_HPP

#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/entities/vtb.hpp>
#include <veriblock/pop/fmt.hpp>

namespace altintegration {

//! @private
struct AddVTB : public Command {
  using block_t = VbkBlock;
  ~AddVTB() override = default;

  template <class VTB_t>
  AddVTB(AltBlockTree& tree, VTB_t&& vtb)
      : tree_(tree), vtb_(std::forward<VTB_t>(vtb)) {}

  bool Execute(ValidationState& state) noexcept override {
    // add commands to the containing VBK block
    return tree_.vbk().addPayloads(
        vtb_.containingBlock.getHash(), {vtb_}, state);
  }

  void UnExecute() noexcept override {
    return tree_.vbk().unsafelyRemovePayload(vtb_.containingBlock.getHash(),
                                             vtb_.getId());
  }

 private:
  AltBlockTree& tree_;
  VTB vtb_;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ADDVTB_HPP
