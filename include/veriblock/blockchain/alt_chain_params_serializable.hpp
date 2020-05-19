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

  double startOfSlope() const noexcept override { return startOfSlope_; }

  double slopeNormal() const noexcept override { return slopeNormal_; }

  double slopeKeystone() const noexcept override { return slopeKeystone_; }

  static PopRewardsCurveParamsSerializable fromRaw(ReadStream& stream);

  static PopRewardsCurveParamsSerializable fromRaw(
      const std::vector<uint8_t>& bytes);

 private:
  double startOfSlope_;
  double slopeNormal_;
  double slopeKeystone_;
};

struct PopRewardsParamsSerializable : public PopRewardsParams {
  ~PopRewardsParamsSerializable() override = default;

  uint32_t keystoneRound() const noexcept override { return keystoneRound_; }

  uint32_t payoutRounds() const noexcept override { return payoutRounds_; }

  uint32_t flatScoreRound() const noexcept override { return flatScoreRound_; }

  bool flatScoreRoundUse() const noexcept override {
    return flatScoreRoundUse_;
  }

  const std::vector<double>& roundRatios() const noexcept override {
    return roundRatios_;
  }

  double maxScoreThresholdNormal() const noexcept override { return 2.0; }

  // limit block with keystones score to this value
  double maxScoreThresholdKeystone() const noexcept override { return 3.0; }

  // collect this amount of blocks BEFORE the block to calculate pop difficulty
  uint32_t difficultyAveragingInterval() const noexcept override { return 50; }

  // wait for this number of blocks before calculating and paying pop reward
  uint32_t rewardSettlementInterval() const noexcept override { return 400; }

  // getter for reward curve parameters
  const PopRewardsCurveParams& getCurveParams() const noexcept override {
    return *curveParams;
  }

  // reward score table
  // we score each VeriBlock and lower the reward for late blocks
  const std::vector<double>& relativeScoreLookupTable()
      const noexcept override {
    return lookupTable_;
  }

  static PopRewardsParamsSerializable fromRaw(ReadStream& stream);

  static PopRewardsParamsSerializable fromRaw(
      const std::vector<uint8_t>& bytes);

 private:
  uint32_t keystoneRound_;
  uint32_t payoutRounds_;
  uint32_t flatScoreRound_;
  bool flatScoreRoundUse_;
  std::vector<double> roundRatios_;
  double maxScoreThresholdNormal_;
  double maxScoreThresholdKeystone_;
  uint32_t difficultyAveragingInterval_;
  uint32_t rewardSettlementInterval_;
  std::vector<double> relativeScoreLookupTable_;
};

}  // namespace altintegration

#endif
