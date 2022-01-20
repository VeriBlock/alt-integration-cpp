// Copyright (c) 2019-2022 Xenios SEZC
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
  bool canFit(const ATV& p) const {
    if (atvs >= params_.getMaxATVsInAltBlock()) {
      return false;
    }
    size_t size = p.estimateSize();
    return canFitSize(size);
  }

  bool canFit(const VTB& p) const {
    if (vtbs >= params_.getMaxVTBsInAltBlock()) {
      return false;
    }
    size_t size = p.estimateSize();
    return canFitSize(size);
  }

  bool canFit(const VbkBlock& p) const {
    if (vbks >= params_.getMaxVbkBlocksInAltBlock()) {
      return false;
    }
    size_t size = p.estimateSize();
    return canFitSize(size);
  }

  void update(const ATV& p) {
    atvs_size += p.estimateSize();
    atvs++;
  }

  void update(const VTB& p) {
    vtbs_size += p.estimateSize();
    vtbs++;
  }

  void update(const VbkBlock& p) {
    vbks_size += p.estimateSize();
    vbks++;
  }

 private:
  bool canFitSize(size_t size) const {
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
