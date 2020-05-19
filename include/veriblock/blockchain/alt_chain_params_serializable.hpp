// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_SERIALIZABLE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_SERIALIZABLE_HPP_

#include <veriblock/blockchain/alt_chain_params.hpp>

namespace altintegration {

struct PopRewardsCurveParamsSerializable : public PopRewardsCurveParams {
  ~PopRewardsCurveParamsSerializable() override = default;

  // we start decreasing rewards after this score
  double startOfSlope() const noexcept override { return startOfSlope_; }

  // we decrease reward coefficient for this value for
  // each additional score point above startOfDecreasingLine
  double slopeNormal() const noexcept override { return slopeNormal_; }

  // slope for keystone rounds
  double slopeKeystone() const noexcept override { return slopeKeystone_; }

  static PopRewardsCurveParamsSerializable fromRaw(ReadStream& stream);

  static PopRewardsCurveParamsSerializable fromRaw(
      const std::vector<uint8_t>& bytes);

 private:
  double startOfSlope_;
  double slopeNormal_;
  double slopeKeystone_;
};

}  // namespace altintegration

#endif
