#ifndef ALTINTEGRATION_ALT_CHAIN_PARAMS_REGTEST_HPP
#define ALTINTEGRATION_ALT_CHAIN_PARAMS_REGTEST_HPP

#include <veriblock/blockchain/alt_chain_params.hpp>

namespace altintegration {

struct AltChainParamsRegTest : public AltChainParams {
  ~AltChainParamsRegTest() override = default;

  AltBlock getBootstrapBlock() const noexcept override {
    AltBlock b;
    b.hash = {1, 2, 3};
    b.height = 0;
    b.timestamp = 0;
    return b;
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ALT_CHAIN_PARAMS_REGTEST_HPP
