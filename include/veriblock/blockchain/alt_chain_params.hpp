#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_

#include <vector>

#include "veriblock/entities/altblock.hpp"

namespace altintegration {

struct AltChainParams {
  virtual ~AltChainParams() = default;

  virtual uint32_t getKeystoneInterval() const noexcept { return 5; }

  ///! number of blocks in VBK for finalization
  virtual uint32_t getFinalityDelay() const noexcept { return 100; }

  virtual std::vector<uint32_t> getForkResolutionLookUpTable() const noexcept {
    // TODO(warchant): this should be recalculated. see paper.
    return {100, 100, 95, 89, 80, 69, 56, 40, 21};
  }

  virtual AltBlock getBootstrapBlock() const noexcept = 0;
};

}  // namespace altintegration

#endif
