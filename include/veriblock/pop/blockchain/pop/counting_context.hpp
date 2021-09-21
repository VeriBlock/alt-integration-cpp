// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CONTINUE_ON_INVALID_HPP
#define VERIBLOCK_POP_CPP_CONTINUE_ON_INVALID_HPP

#include <veriblock/pop/blockchain/alt_chain_params.hpp>
#include <veriblock/pop/blockchain/command_group.hpp>
#include <veriblock/pop/entities/popdata.hpp>

namespace altintegration {

//! @private
struct CountingContext {
  CountingContext(const AltChainParams& params) : params_(params) {}

  //! returns true if given command group can fit into a block
  bool canFit(const ATV& atv) {
    if (atvs >= params_.getMaxATVsInAltBlock()) {
      return false;
    }
    size_t atv_size = atv.estimateSize();
    if (!canFitSize(atv_size)) {
      return false;
    }

    atvs_size += atv_size;
    atvs++;
    return true;
  }

  bool canFit(const VTB& vtb) {
    if (vtbs >= params_.getMaxVTBsInAltBlock()) {
      return false;
    }
    size_t vtb_size = vtb.estimateSize();
    if (!canFitSize(vtb_size)) {
      return false;
    }

    vtbs_size += vtb_size;
    vtbs++;
    return true;
  }

  bool canFit(const VbkBlock& vbk) {
    if (vbks >= params_.getMaxVbkBlocksInAltBlock()) {
      return false;
    }
    size_t vbk_size = vbk.estimateSize();
    if (!canFitSize(vbk_size)) {
      return false;
    }

    vbks_size += vbk_size;
    vbks++;
    return true;
  }

 private:
  bool canFitSize(size_t size) {
    // clang-format off
    size_t popdatasize =
      sizeof(PopData::version) +
      singleBEValueSize(atvs) + atvs_size +
      singleBEValueSize(vtbs) + vtbs_size +
      singleBEValueSize(vbks) + vbks_size;
    // clang-format on
    return popdatasize + size <= params_.getMaxPopDataSize();
  }

 private:
  size_t atvs = 0;
  size_t vtbs = 0;
  size_t vbks = 0;
  size_t atvs_size = 0;
  size_t vtbs_size = 0;
  size_t vbks_size = 0;
  const AltChainParams& params_;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_CONTINUE_ON_INVALID_HPP
