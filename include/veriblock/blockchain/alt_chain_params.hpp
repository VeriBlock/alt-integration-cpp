#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_

#include <vector>

#include "veriblock/entities/altblock.hpp"

namespace altintegration {

struct AltChainParams {
  virtual ~AltChainParams() = default;

  virtual uint32_t getKeystoneInterval() const noexcept { return 5; }

  virtual uint32_t getFinalityDelay() const noexcept { return 50; }

  virtual std::vector<uint32_t> getForkResolutionLookUpTable() const noexcept {
    return {100, 100, 95, 89, 80, 69, 56, 40, 21};
  }

  virtual AltBlock getGenesisBlock() const noexcept {
    return {{1, 2, 3}, {4, 5, 6}, 0, 0};
  }
};

}  // namespace altintegration

#endif
