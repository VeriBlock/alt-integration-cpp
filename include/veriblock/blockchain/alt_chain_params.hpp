#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_

#include <vector>

namespace altintegration {

struct AltChainParams {
  virtual ~AltChainParams() = default;

  virtual uint32_t getKeystoneInterval() const noexcept { return 5; }

  virtual uint32_t getFinalityDelay() const noexcept { return 50; }

  virtual std::vector<uint32_t> getForkResolutionLookUpTable() const noexcept {
    return {100, 100, 95, 89, 80, 69, 56, 40, 21};
  }
};

}  // namespace altintegration

#endif
