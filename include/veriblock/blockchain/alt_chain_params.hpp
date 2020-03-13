#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_

#include <vector>

namespace VeriBlock {

struct AltChainParams {
  virtual uint32_t getKeystoneInterval() const noexcept;

  virtual uint32_t getFinalityDelay() const noexcept;

  virtual std::vector<uint32_t> getForkResolutionLookUpTable() const noexcept;
};

}  // namespace VeriBlock

#endif
