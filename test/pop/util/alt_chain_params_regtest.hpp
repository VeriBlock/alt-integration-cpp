// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ALT_CHAIN_PARAMS_REGTEST_HPP
#define ALTINTEGRATION_ALT_CHAIN_PARAMS_REGTEST_HPP

#include <veriblock/pop/blockchain/alt_chain_params.hpp>

namespace altintegration {

struct AltChainParamsRegTest : public AltChainParams {
  AltChainParamsRegTest(int id = 0) : id(id) {}
  ~AltChainParamsRegTest() override = default;

  AltBlock getBootstrapBlock() const noexcept override {
    AltBlock b;
    b.hash = std::vector<uint8_t>(12, 1);
    b.previousBlock = std::vector<uint8_t>(12, 0);
    b.height = 0;
    b.timestamp = 0;
    return b;
  }

  int64_t getIdentifier() const noexcept override { return id; }

  std::vector<uint8_t> getHash(
      const std::vector<uint8_t>& bytes) const noexcept override {
    return AssertDeserializeFromRaw<AltBlock>(bytes).getHash();
  }

  bool checkBlockHeader(const std::vector<uint8_t>&,
                        const std::vector<uint8_t>&,
                        ValidationState&) const noexcept override {
    return true;
  }

  int64_t id = 0;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ALT_CHAIN_PARAMS_REGTEST_HPP
