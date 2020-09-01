// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_

#include <string>
#include <vector>

#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/entities/altblock.hpp"
#include "veriblock/serde.hpp"

/**
 * @defgroup config Altchain Config
 * Data structures that store all altchain-related configs required for POP
 * integration.
 */

namespace altintegration {

/**
 * @struct PopRewardsCurveParams
 *
 * Defines POP rewards payout curve parameters.
 * @ingroup config,interfaces
 */
struct PopRewardsCurveParams {
  //! we start decreasing rewards after this score
  double startOfSlope() const noexcept { return mStartOfSlope; }

  //! we decrease reward coefficient for this value for
  //! each additional score point above startOfDecreasingLine
  double slopeNormal() const noexcept { return mSlopeNormal; }

  //! slope for keystone rounds
  double slopeKeystone() const noexcept { return mSlopeKeystone; }

 protected:
  double mStartOfSlope = 1.0;
  double mSlopeNormal = 0.2;
  double mSlopeKeystone = 0.21325;
};

/**
 * @struct PopRewardsParams
 *
 * Defines config for POP rewards.
 * @ingroup config, interfaces
 */
struct PopRewardsParams {
  //! we use this round number to detect keystones
  uint32_t keystoneRound() const noexcept { return mKeystoneRound; }

  //! we have this number of rounds eg rounds 0, 1, 2, 3
  uint32_t payoutRounds() const noexcept { return mPayoutRounds; }

  //! we use this round number to pay flat reward (does not depend on pop
  //! difficulty)
  uint32_t flatScoreRound() const noexcept { return mFlatScoreRound; }

  //! should we use flat rewards at all
  bool useFlatScoreRound() const noexcept { return mUseFlatScoreRound; }

  //! we have these payout modifiers for different rounds. Keystone round has
  //! the highest multiplier
  const std::vector<double>& roundRatios() const noexcept {
    return mRoundRatios;
  }

  //! limit block score to this value
  double maxScoreThresholdNormal() const noexcept {
    return mMaxScoreThresholdNormal;
  }

  //! limit block with keystones score to this value
  double maxScoreThresholdKeystone() const noexcept {
    return mMaxScoreThresholdKeystone;
  }

  //! collect this amount of blocks BEFORE the block to calculate pop difficulty
  uint32_t difficultyAveragingInterval() const noexcept {
    return mDifficultyAveragingInterval;
  }

  //! getter for reward curve parameters
  const PopRewardsCurveParams& getCurveParams() const noexcept {
    return curveParams;
  }

  //! reward score table
  //! we score each VeriBlock and lower the reward for late blocks
  const std::vector<double>& relativeScoreLookupTable() const noexcept {
    return mLookupTable;
  }

 protected:
  PopRewardsCurveParams curveParams{};

  uint32_t mKeystoneRound = 3;
  uint32_t mPayoutRounds = 4;
  uint32_t mFlatScoreRound = 2;
  bool mUseFlatScoreRound = true;
  double mMaxScoreThresholdNormal = 2.0;
  double mMaxScoreThresholdKeystone = 3.0;
  uint32_t mDifficultyAveragingInterval = 50;

  std::vector<double> mRoundRatios{0.97, 1.03, 1.07, 3.00};

  std::vector<double> mLookupTable{
      1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000,
      1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000,
      0.48296816, 0.31551694, 0.23325824, 0.18453616, 0.15238463, 0.12961255,
      0.11265630, 0.09955094, 0.08912509, 0.08063761, 0.07359692, 0.06766428,
      0.06259873, 0.05822428, 0.05440941, 0.05105386, 0.04807993, 0.04542644,
      0.04304458, 0.04089495, 0.03894540, 0.03716941, 0.03554497, 0.03405359,
      0.03267969, 0.03141000, 0.03023319, 0.02913950, 0.02812047, 0.02716878,
      0.02627801, 0.02544253, 0.02465739, 0.02391820, 0.02322107, 0.02256255,
      0.02193952, 0.02134922};
};

/**
 * @ingroup config, interfaces
 *
 * @struct AltChainParams
 *
 * Base class for all Altchain-related configs.
 */
struct AltChainParams {
  virtual ~AltChainParams() = default;

  //! number of blocks in single keystone interval. 5 means that blocks with
  //! heights 5,6,7,8,9 are blocks within same keystone interval
  uint32_t getKeystoneInterval() const noexcept { return mKeystoneInterval; }

  //! number of blocks in VBK for finalization
  uint32_t getFinalityDelay() const noexcept { return mFinalityDelay; }

  //! pop score lookup table for fork resolution
  const std::vector<uint32_t>& getForkResolutionLookUpTable() const noexcept {
    // TODO(warchant): this should be recalculated. see paper.
    return mForkResolutionLookUpTable;
  }

  //! validity window for ATVs; pop payout delay, in blocks
  int32_t getEndorsementSettlementInterval() const noexcept {
    return mEndorsementSettlementInterval;
  }

  //! maximum size of single PopData in a single ALT block, in bytes
  uint32_t getMaxPopDataSize() const noexcept { return mMaxPopDataSize; }

  //! @deprecated
  bool isStrictAddPayloadsOrderingEnabled() const noexcept {
    return mStrictAddPayloadsOrderingEnabled;
  }

  //! getter for reward parameters
  const PopRewardsParams& getRewardParams() const noexcept {
    return mPopRewardsParams;
  }

  //! Maximum future block time for altchain blocks. Must be low enough such
  //! that attacker can't produce endorsements faster than this interval.
  uint32_t maxFutureBlockTime() const noexcept { return mMaxFutureBlockTime; }

  //! unique POP ID for the chain
  virtual int64_t getIdentifier() const noexcept = 0;

  //! first ALT block used in AltBlockTree. This is first block that can be endorsed.
  //! @refitem altbootstrapblock
  virtual AltBlock getBootstrapBlock() const noexcept = 0;

  /**
   * Calculate hash from block header.
   * @param bytes serialized block header
   * @return hash
   */
  virtual std::vector<uint8_t> getHash(
      const std::vector<uint8_t>& bytes) const noexcept = 0;

 protected:
  PopRewardsParams mPopRewardsParams;

  // should be disabled in tests
  // should be enabled in prod
  bool mStrictAddPayloadsOrderingEnabled = false;
  uint32_t mMaxFutureBlockTime = 10 * 60;  // 10 min
  uint32_t mKeystoneInterval = 5;
  uint32_t mFinalityDelay = 100;
  int32_t mEndorsementSettlementInterval = 50;
  uint32_t mMaxPopDataSize = 1 * 1024 * 1024;  // 1 MB

  std::vector<uint32_t> mForkResolutionLookUpTable{
      100, 100, 95, 89, 80, 69, 56, 40, 21};
};

}  // namespace altintegration

#endif
