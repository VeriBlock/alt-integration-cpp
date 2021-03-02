// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_

#include <string>
#include <vector>
#include <veriblock/pop/blockchain/block_index.hpp>
#include <veriblock/pop/entities/altblock.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/validation_state.hpp>

/**
 * @defgroup config Altchain Config
 * Data structures that store all altchain-related configs required for POP
 * integration.
 */

namespace altintegration {

/**
 * @struct PopPayoutParams
 *
 * Defines config for POP payouts.
 * @ingroup config, interfaces
 */
struct PopPayoutsParams {
  //! we start decreasing rewards after this score
  double startOfSlope() const noexcept { return mStartOfSlope; }

  //! we decrease reward coefficient for this value for
  //! each additional score point above startOfDecreasingLine
  double slopeNormal() const noexcept { return mSlopeNormal; }

  //! slope for keystone rounds
  double slopeKeystone() const noexcept { return mSlopeKeystone; }

  //! among all rounds, this number represents round for keystone blocks.
  //! we use this round number to detect keystones.
  uint32_t keystoneRound() const noexcept { return mKeystoneRound; }

  //! total number of payout rounds.
  //! we have this number of rounds eg rounds 0, 1, 2, 3, total is 4
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

  //! reward score table
  //! we score each VeriBlock and lower the reward for late blocks
  const std::vector<double>& relativeScoreLookupTable() const noexcept {
    return mLookupTable;
  }

  //! number of blocks in ALT between endorsed block and payout block
  int32_t getPopPayoutDelay() const noexcept { return mPopPayoutDelay; }

  // it is public for C wrapper
 public:
  double mStartOfSlope = 1.0;
  double mSlopeNormal = 0.2;
  double mSlopeKeystone = 0.21325;
  uint32_t mKeystoneRound = 3;
  uint32_t mPayoutRounds = 4;
  uint32_t mFlatScoreRound = 2;
  bool mUseFlatScoreRound = true;
  double mMaxScoreThresholdNormal = 2.0;
  double mMaxScoreThresholdKeystone = 3.0;
  uint32_t mDifficultyAveragingInterval = 50;
  int32_t mPopPayoutDelay = 50;

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

template <typename JsonValue>
JsonValue ToJSON(const PopPayoutsParams& p) {
  auto obj = json::makeEmptyObject<JsonValue>();
  json::putArrayKV(obj, "lookupTable", p.relativeScoreLookupTable());
  json::putArrayKV(obj, "roundRatios", p.roundRatios());
  json::putIntKV(
      obj, "difficultyAveragingInterval", p.difficultyAveragingInterval());
  json::putDoubleKV(
      obj, "maxScoreThresholdKeystone", p.maxScoreThresholdKeystone());
  json::putDoubleKV(
      obj, "maxScoreThresholdNormal", p.maxScoreThresholdNormal());
  json::putBoolKV(obj, "useFlatScoreRound", p.useFlatScoreRound());
  json::putIntKV(obj, "flatScoreRound", p.flatScoreRound());
  json::putIntKV(obj, "payoutRounds", p.payoutRounds());
  json::putIntKV(obj, "keystoneRound", p.keystoneRound());
  json::putDoubleKV(obj, "slopeKeystone", p.slopeKeystone());
  json::putDoubleKV(obj, "slopeNormal", p.slopeNormal());
  json::putDoubleKV(obj, "startOfSlope", p.startOfSlope());
  json::putIntKV(obj, "popPayoutDelay", p.getPopPayoutDelay());
  return obj;
}

/**
 * @ingroup config, interfaces
 *
 * @struct AltChainParams
 *
 * Base class for all Altchain-related configs.
 */
struct AltChainParams {
  virtual ~AltChainParams() = default;

  size_t maxWorkerQueueSize() const noexcept {
    const auto ret = getMaxATVsInAltBlock() + getMaxVTBsInAltBlock() +
                     getMaxVbkBlocksInAltBlock();
    static const auto MAX = 400000;
    VBK_ASSERT_MSG(ret < MAX,
                   "Worker size queue can not be more than %d, otherwise it "
                   "would take more than 50MB of RAM",
                   MAX);
    return ret;
  }

  //! number of blocks in single keystone interval. 5 means that blocks with
  //! heights 5,6,7,8,9 are blocks within same keystone interval
  uint32_t getKeystoneInterval() const noexcept { return mKeystoneInterval; }

  //! number of blocks in VBK used for finalization
  uint32_t getFinalityDelay() const noexcept { return mFinalityDelay; }

  //! pop score lookup table for fork resolution
  const std::vector<uint32_t>& getForkResolutionLookUpTable() const noexcept {
    // TODO(warchant): this should be recalculated. see paper.
    return mForkResolutionLookUpTable;
  }

  //! Validity window for ATVs. If difference between endorsed/containing blocks
  //! is more than this number, endorsement becomes invalid.
  int32_t getEndorsementSettlementInterval() const noexcept {
    VBK_ASSERT(mEndorsementSettlementInterval >= 1);
    return mEndorsementSettlementInterval;
  }

  //! getter for reward parameters
  const PopPayoutsParams& getPayoutParams() const noexcept {
    return *mPopPayoutsParams;
  }

  //! total maximum number of VBK blocks per 1 ALT block
  size_t getMaxVbkBlocksInAltBlock() const noexcept {
    return mMaxVbkBlocksInAltBlock;
  }

  //! total maximum number of VTBs per 1 ALT block
  size_t getMaxVTBsInAltBlock() const noexcept { return mMaxVTBsInAltBlock; }

  //! total maximum number of ATVs per 1 ALT block
  size_t getMaxATVsInAltBlock() const noexcept { return mMaxATVsInAltBlock; }

  //! maximum size (in bytes) of single PopData per single ALT block
  size_t getMaxPopDataSize() const noexcept { return mMaxPopDataSize; }

  //! Maximum future block time for altchain blocks.
  uint32_t maxAltchainFutureBlockTime() const noexcept {
    return mMaxAltchainFutureBlockTime;
  }

  //! unique POP ID for the chain; identifies altchain in VBK
  virtual int64_t getIdentifier() const noexcept = 0;

  //! "genesis" block for POP mining. This is first block that can be
  //! endorsed
  //! by POP miners.
  //! Can start at genesis block, or at any part of the active chain. This block
  //! is immediately finalized, so chain CAN NOT be reorganized past this block.
  virtual AltBlock getBootstrapBlock() const noexcept = 0;

  /**
   * Calculate hash from block header.
   * @param bytes serialized block header
   * @return hash
   * @note if input data is not valid block header, still calculate hash from
   * input data.
   *
   * @warning SHOULD NOT THROW
   */
  virtual std::vector<uint8_t> getHash(
      const std::vector<uint8_t>& bytes) const noexcept = 0;

  /**
   * Returns true if input `bytes`:
   *  - can be deserialized to a header
   *  - statelessly valid (time, POW, version... etc)
   *  - top level merkle root is equal to 'root'
   * @param bytes
   * @param root top level merkle root to validate
   * @return false if any of checks fail
   *
   * @warning SHOULD NOT THROW
   */
  virtual bool checkBlockHeader(const std::vector<uint8_t>& bytes,
                                const std::vector<uint8_t>& root,
                                ValidationState& state) const noexcept = 0;

 public:
  std::shared_ptr<PopPayoutsParams> mPopPayoutsParams =
      std::make_shared<PopPayoutsParams>();

  uint32_t mMaxAltchainFutureBlockTime = 10 * 60;  // 10 min
  uint32_t mKeystoneInterval = 5;
  uint32_t mFinalityDelay = 100;
  int32_t mEndorsementSettlementInterval = 50;
  uint32_t mMaxPopDataSize = MAX_POPDATA_SIZE;

  size_t mMaxVbkBlocksInAltBlock = 200;
  size_t mMaxVTBsInAltBlock = 200;
  size_t mMaxATVsInAltBlock = 1000;

  std::vector<uint32_t> mForkResolutionLookUpTable{
      100, 100, 95, 89, 80, 69, 56, 40, 21};
};

template <typename JsonValue>
JsonValue ToJSON(const AltChainParams& p, bool reverseAltHashes = true) {
  auto obj = json::makeEmptyObject<JsonValue>();
  json::putIntKV(obj, "networkId", p.getIdentifier());
  json::putArrayKV(
      obj, "forkResolutionLookupTable", p.getForkResolutionLookUpTable());
  json::putIntKV(obj, "maxVbkBlocksInAltBlock", p.getMaxVbkBlocksInAltBlock());
  json::putIntKV(obj, "maxVTBsInAltBlock", p.getMaxVTBsInAltBlock());
  json::putIntKV(obj, "maxATVsInAltBlock", p.getMaxATVsInAltBlock());
  json::putIntKV(obj,
                 "endorsementSettlementInterval",
                 p.getEndorsementSettlementInterval());
  json::putIntKV(obj, "finalityDelay", p.getFinalityDelay());
  json::putIntKV(obj, "keystoneInterval", p.getKeystoneInterval());
  json::putIntKV(
      obj, "maxAltchainFutureBlockTime", p.maxAltchainFutureBlockTime());
  json::putKV(obj, "payoutParams", ToJSON<JsonValue>(p.getPayoutParams()));
  json::putKV(obj,
              "bootstrapBlock",
              ToJSON<JsonValue>(p.getBootstrapBlock(), reverseAltHashes));
  return obj;
}

}  // namespace altintegration

#endif