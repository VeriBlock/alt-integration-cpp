// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CONTINUE_ON_INVALID_HPP
#define VERIBLOCK_POP_CPP_CONTINUE_ON_INVALID_HPP

#include <veriblock/pop/blockchain/alt_chain_params.hpp>
#include <veriblock/pop/blockchain/command_group.hpp>
#include <veriblock/pop/entities/atv.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/entities/vtb.hpp>

namespace altintegration {

//! @private
struct ContinueOnInvalidContext {
  ContinueOnInvalidContext(const AltChainParams& params) : params_(params) {}

  //! returns true if given command group can fit into a block
  bool canFit(const ATV&) {
    if (atvs > params_.getMaxATVsInAltBlock()) {
      return false;
    }

    atvs++;
    return true;
  }

  bool canFit(const VTB&) {
    if (vtbs > params_.getMaxVTBsInAltBlock()) {
      return false;
    }

    vtbs++;
    return true;
  }

  bool canFit(const VbkBlock&) {
    if (vbks > params_.getMaxVbkBlocksInAltBlock()) {
      return false;
    }

    vbks++;
    return true;
  }

 private:
  size_t vtbs = 0;
  size_t atvs = 0;
  size_t vbks = 0;
  const AltChainParams& params_;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_CONTINUE_ON_INVALID_HPP
