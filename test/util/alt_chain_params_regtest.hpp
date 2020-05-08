// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ALT_CHAIN_PARAMS_REGTEST_HPP
#define ALTINTEGRATION_ALT_CHAIN_PARAMS_REGTEST_HPP

#include <veriblock/blockchain/alt_chain_params.hpp>

namespace altintegration {

struct AltChainParamsRegTest : public AltChainParams {
  AltChainParamsRegTest(int id = 0) : id(id) {}
  ~AltChainParamsRegTest() override = default;

  AltBlock getBootstrapBlock() const noexcept override {
    AltBlock b;
    b.hash = {1, 2, 3};
    b.height = 0;
    b.timestamp = 0;
    return b;
  }

  uint32_t getIdentifier() const noexcept override { return id; }

  uint32_t id = 0;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ALT_CHAIN_PARAMS_REGTEST_HPP
