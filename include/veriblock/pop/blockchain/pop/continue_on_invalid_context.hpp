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
  bool canFit(const ATV& atv) {
    if (atvs >= params_.getMaxATVsInAltBlock()) {
      return false;
    }
    size_t atv_size = atv.estimateSize();
    if (pop_size + atv_size > params_.getMaxPopDataSize()) {
      return false;
    }

    pop_size += atv_size;
    atvs++;
    return true;
  }

  bool canFit(const VTB& vtb) {
    if (vtbs >= params_.getMaxVTBsInAltBlock()) {
      return false;
    }
    size_t vtb_size = vtb.estimateSize();
    if (pop_size + vtb_size > params_.getMaxPopDataSize()) {
      return false;
    }

    pop_size += vtb_size;
    vtbs++;
    return true;
  }

  bool canFit(const VbkBlock& vbk) {
    if (vbks >= params_.getMaxVbkBlocksInAltBlock()) {
      return false;
    }
    size_t vbk_size = vbk.estimateSize();
    if (pop_size + vbk_size > params_.getMaxPopDataSize()) {
      return false;
    }

    pop_size += vbk_size;
    vbks++;
    return true;
  }

 private:
  size_t vtbs = 0;
  size_t atvs = 0;
  size_t vbks = 0;
  size_t pop_size = PopData{}.estimateSize() + 20;
  const AltChainParams& params_;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_CONTINUE_ON_INVALID_HPP
