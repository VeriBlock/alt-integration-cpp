// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CONTINUE_ON_INVALID_HPP
#define VERIBLOCK_POP_CPP_CONTINUE_ON_INVALID_HPP

#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/blockchain/command_group.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/entities/vtb.hpp>

namespace altintegration {

struct ContinueOnInvalidContext {
  ContinueOnInvalidContext(const AltChainParams& params) : params_(params) {}

  bool canFit(const CommandGroup& g) noexcept {
    if (g.payload_type_name == &ATV::name()) {
      if (atvs >= params_.getMaxATVsInAltBlock()) {
        return false;
      }

      atvs++;
      return true;
    }

    if (g.payload_type_name == &VTB::name()) {
      if (vtbs >= params_.getMaxVTBsInAltBlock()) {
        return false;
      }

      vtbs++;
      return true;
    }

    if (g.payload_type_name == &VbkBlock::name()) {
      if (vbks >= params_.getMaxVbkBlocksInAltBlock()) {
        return false;
      }

      vbks++;
      return true;
    }

    VBK_ASSERT_MSG(false,
                   "command group contains bad typename: %s",
                   (g.payload_type_name ? *g.payload_type_name : "nullptr"));
    return false;
  }

 private:
  size_t vtbs = 0;
  size_t atvs = 0;
  size_t vbks = 0;
  const AltChainParams& params_;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_CONTINUE_ON_INVALID_HPP
