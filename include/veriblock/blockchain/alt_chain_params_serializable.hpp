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

  double maxScoreThresholdNormal() const noexcept override {
    return maxScoreThresholdNormal_;
  }

  double maxScoreThresholdKeystone() const noexcept override {
    return maxScoreThresholdKeystone_;
  }

  uint32_t difficultyAveragingInterval() const noexcept override {
    return difficultyAveragingInterval_;
  }

  const PopRewardsCurveParams& getCurveParams() const noexcept override {
    return *curveParams;
  }
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
  std::vector<double> lookupTable_;
};

struct AltChainParamsSerializable : public AltChainParams {
  ~AltChainParamsSerializable() override = default;

  uint32_t getKeystoneInterval() const noexcept override {
    return keystoneInterval_;
  }

  uint32_t getFinalityDelay() const noexcept override { return finalityDelay_; }

  const std::vector<uint32_t>& getForkResolutionLookUpTable()
      const noexcept override {
    return forkResolutionLookUpTable_;
  }

  int32_t getEndorsementSettlementInterval() const noexcept override {
    return endorsementSettlementInterval_;
  }

  uint32_t getMaxPopDataPerBlock() const noexcept override {
    return maxPopDataPerBlock_;
  };

  uint32_t getMaxPopDataWeight() const noexcept override {
    return maxPopDataWeight_;
  };

  uint32_t getSuperMaxPopDataWeight() const noexcept override {
    return superMaxPopDataWeight_;
  };

  const PopRewardsParams& getRewardParams() const noexcept override {
    return *popRewardsParams;
  }

  uint32_t getIdentifier() const noexcept override { return indentifier_; };

  AltBlock getBootstrapBlock() const noexcept override {
    return bootstrapBlock_;
  };

  std::vector<uint8_t> getHash(
      const std::vector<uint8_t>&) const noexcept override {
    return std::vector<uint8_t>();
  }

  static AltChainParamsSerializable fromRaw(ReadStream& stream);

  static AltChainParamsSerializable fromRaw(const std::vector<uint8_t>& bytes);

 private:
  uint32_t keystoneInterval_;
  uint32_t finalityDelay_;
  std::vector<uint32_t> forkResolutionLookUpTable_;
  int32_t endorsementSettlementInterval_;
  uint32_t maxPopDataPerBlock_;
  uint32_t maxPopDataWeight_;
  uint32_t superMaxPopDataWeight_;
  uint32_t indentifier_;
  AltBlock bootstrapBlock_;
};

}  // namespace altintegration

#endif
