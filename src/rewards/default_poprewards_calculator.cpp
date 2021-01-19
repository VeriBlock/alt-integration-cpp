// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <cassert>
#include <vector>
#include <veriblock/entities/atv.hpp>
#include <veriblock/exceptions/state_corrupted.hpp>
#include <veriblock/rewards/default_poprewards_calculator.hpp>

namespace altintegration {

static int getBestPublicationHeight(const BlockIndex<AltBlock>& endorsedBlock,
                                    const VbkBlockTree& vbk_tree) {
  int bestPublication = -1;
  for (const auto* e : endorsedBlock.endorsedBy) {
    auto* b = vbk_tree.getBlockIndex(e->blockOfProof);
    if (!vbk_tree.getBestChain().contains(b)) continue;
    if (b->getHeight() < bestPublication || bestPublication < 0)
      bestPublication = b->getHeight();
  }
  return bestPublication;
}

static bool isKeystoneRound(const PopPayoutsParams& rewardParams,
                            uint32_t payoutRound) {
  return payoutRound == rewardParams.keystoneRound();
}

// test if block is placed inside payout round following keystone
static bool isFirstRoundAfterKeystone(const AltChainParams& altParams,
                                      uint32_t height) {
  uint32_t blockAfterKeystone = height % altParams.getKeystoneInterval();
  if (altParams.getPayoutParams().payoutRounds() == 0) {
    return true;
  }
  return blockAfterKeystone / altParams.getPayoutParams().payoutRounds() == 0;
}

static PopRewardsBigDecimal getRoundRatio(const PopPayoutsParams& rewardParams,
                                          uint32_t payoutRound) {
  return rewardParams.roundRatios().at(payoutRound);
}

static PopRewardsBigDecimal getMaxScoreThreshold(
    const PopPayoutsParams& rewardParams, uint32_t payoutRound) {
  if (isKeystoneRound(rewardParams, payoutRound)) {
    return rewardParams.maxScoreThresholdKeystone();
  }
  return rewardParams.maxScoreThresholdNormal();
}

// slope is how the payout is decreased for each additional block score
static PopRewardsBigDecimal getRoundSlope(const PopPayoutsParams& params,
                                          uint32_t payoutRound) {
  auto slopeRatio = params.slopeNormal();
  if (payoutRound == params.keystoneRound()) {
    slopeRatio = params.slopeKeystone();
  }
  return slopeRatio;
}

// apply the reward curve to the score and subtract it from the current round
// multiplier
static PopRewardsBigDecimal calculateSlopeRatio(
    const PopPayoutsParams& params,
    const PopRewardsBigDecimal& score,
    uint32_t payoutRound) {
  const auto& slope = getRoundSlope(params, payoutRound);
  VBK_ASSERT(score >= params.startOfSlope());

  auto scoreDecrease = slope * (score - params.startOfSlope());
  PopRewardsBigDecimal maxScoreDecrease = 1.0;
  if (scoreDecrease > maxScoreDecrease) {
    scoreDecrease = maxScoreDecrease;
  }
  return (maxScoreDecrease - scoreDecrease);
}

static std::vector<const DefaultPopRewardsCalculator::index_t*>
fetchBlocksUntil(const DefaultPopRewardsCalculator::index_t* from,
                 const DefaultPopRewardsCalculator::index_t* check,
                 size_t count) {
  std::vector<const DefaultPopRewardsCalculator::index_t*> blocks;
  blocks.reserve(count);
  const DefaultPopRewardsCalculator::index_t* curBlock = from;

  for (size_t i = 0; i < count; i++) {
    if (curBlock == nullptr) break;
    if (curBlock == check) break;
    blocks.push_back(curBlock);
    curBlock = curBlock->pprev;
  }
  return blocks;
}

// rounds for blocks are [3, 1, 2, 0, 1, 2, 0, 1, 2, 0, 3, ...]
uint32_t DefaultPopRewardsCalculator::getRoundForBlockNumber(
    uint32_t height) const {
  const auto ki = tree_.getParams().getKeystoneInterval();
  const PopPayoutsParams& params = tree_.getParams().getPayoutParams();
  if (isKeystone(height, ki)) {
    return params.keystoneRound();
  }

  if (params.payoutRounds() <= 1) {
    return 0;
  }

  VBK_ASSERT(height > 0);
  uint32_t round = (height % ki) % (params.payoutRounds() - 1);
  return round;
}

PopRewardsBigDecimal
DefaultPopRewardsCalculator::getScoreMultiplierFromRelativeBlock(
    int relativeBlock) const {
  auto table = tree_.getParams().getPayoutParams().relativeScoreLookupTable();
  if (relativeBlock < 0 || relativeBlock >= static_cast<int>(table.size())) {
    return 0.0;
  }

  return table[relativeBlock];
}

PopRewardsBigDecimal DefaultPopRewardsCalculator::calculateBlockReward(
    uint32_t height,
    PopRewardsBigDecimal popscore,
    PopRewardsBigDecimal popdifficulty) const {
  const auto& params = tree_.getParams().getPayoutParams();
  uint32_t payoutRound = getRoundForBlockNumber(height);

  if (params.useFlatScoreRound() && payoutRound == params.flatScoreRound() &&
      isFirstRoundAfterKeystone(tree_.getParams(), height)) {
    popscore = 1.0;
    popdifficulty = 1.0;
  }

  if (popscore == 0.0) {
    return 0.0;
  }

  // Minimum difficulty
  if (popdifficulty < 1.0) {
    popdifficulty = 1.0;
  }

  auto scoreToDifficulty = popscore / popdifficulty;
  auto roundRatio = getRoundRatio(params, payoutRound);

  // penalty multiplier
  PopRewardsBigDecimal slope = 1.0;

  if (scoreToDifficulty > params.startOfSlope()) {
    auto maxScoreThreshold = getMaxScoreThreshold(params, payoutRound);
    if (scoreToDifficulty > maxScoreThreshold) {
      scoreToDifficulty = maxScoreThreshold;
    }

    // Note that this reward per point is not the true rewardWithSlope if the
    // score to difficulty ratio is greater than the max reward threshold. Past
    // the max reward threshold, the block reward ceases to grow, but is split
    // amongst a larger number of participants.
    slope = calculateSlopeRatio(params, scoreToDifficulty, payoutRound);
  }

  return slope * scoreToDifficulty * roundRatio;
}

PopRewardsBigDecimal DefaultPopRewardsCalculator::calculateMinerReward(
    uint32_t vbkRelativeHeight,
    const PopRewardsBigDecimal& scoreForThisBlock,
    const PopRewardsBigDecimal& blockReward) const {
  if (scoreForThisBlock == 0.0) {
    return 0.0;
  }
  auto endorsementLevelWeight =
      getScoreMultiplierFromRelativeBlock(vbkRelativeHeight);
  return blockReward * endorsementLevelWeight / scoreForThisBlock;
}

PopRewardsBigDecimal DefaultPopRewardsCalculator::scoreFromEndorsements(
    const BlockIndex<AltBlock>& endorsedBlock) {
  const auto it = cache_.find(&endorsedBlock);
  if (it != cache_.end()) {
    return it->second;
  }

  PopRewardsBigDecimal totalScore = 0.0;
  // we simply find the lowest VBK height in the endorsements
  int bestPublication = getBestPublicationHeight(endorsedBlock, tree_.vbk());
  if (bestPublication < 0) return totalScore;

  for (const auto* e : endorsedBlock.endorsedBy) {
    auto* b = tree_.vbk().getBlockIndex(e->blockOfProof);
    if (!tree_.vbk().getBestChain().contains(b)) continue;
    int relativeHeight = b->getHeight() - bestPublication;
    assert(relativeHeight >= 0);
    totalScore += getScoreMultiplierFromRelativeBlock(relativeHeight);
  }
  return totalScore;
}

PopRewardsBigDecimal DefaultPopRewardsCalculator::calculateDifficulty(
    const BlockIndex<AltBlock>& tip) {
  PopRewardsBigDecimal difficulty = 0.0;
  auto& params = tree_.getParams().getPayoutParams();
  const BlockIndex<AltBlock>* currentBlock = tip.pprev;

  for (size_t i = 0; i < params.difficultyAveragingInterval(); i++) {
    if (currentBlock == nullptr) break;
    difficulty += scoreFromEndorsements(*currentBlock);
    currentBlock = currentBlock->pprev;
  }

  difficulty /= static_cast<uint64_t>(params.difficultyAveragingInterval());

  // Minimum difficulty
  if (difficulty < 1.0) {
    difficulty = 1.0;
  }
  return difficulty;
}

PopPayouts DefaultPopRewardsCalculator::calculatePayoutsInner(
    const BlockIndex<AltBlock>& endorsedBlock,
    const PopRewardsBigDecimal& endorsedBlockScore,
    const PopRewardsBigDecimal& popDifficulty) {
  PopPayouts rewards{};
  int bestPublication = getBestPublicationHeight(endorsedBlock, tree_.vbk());
  if (bestPublication < 0) {
    return rewards;
  }

  // precalculate block reward - it helps calculating each miner's reward
  auto blockReward = calculateBlockReward(
      endorsedBlock.getHeight(), endorsedBlockScore, popDifficulty);

  // pay reward for each of the endorsements
  for (const auto* e : endorsedBlock.endorsedBy) {
    VBK_ASSERT(e != nullptr);
    auto* b = tree_.vbk().getBlockIndex(e->blockOfProof);
    if (!tree_.vbk().getBestChain().contains(b)) {
      continue;
    }

    ATV atv;
    ValidationState state;
    if (!tree_.getPayloadsProvider().getPayloadsReader().getATV(
            e->getId(), atv, state)) {
      state.Invalid(fmt::format("cant-load-atv-{}", HexStr(e->getId())));
      throw StateCorruptedException(endorsedBlock, state);
    }

    int veriBlockHeight = b->getHeight();
    int relativeHeight = veriBlockHeight - bestPublication;
    assert(relativeHeight >= 0);
    auto minerReward =
        calculateMinerReward(relativeHeight, endorsedBlockScore, blockReward);
    auto payoutInfo = atv.transaction.publicationData.payoutInfo;
    rewards.payouts[payoutInfo] += minerReward.value.getLow64();
  }
  return rewards;
}

PopRewardsBigDecimal DefaultPopRewardsCalculator::appendToCache(
    const index_t& block) {
  const index_t* frontBlock = nullptr;
  const index_t* backBlock = nullptr;
  if (!history_.empty()) {
    frontBlock = history_.front();
    backBlock = history_.back();
    VBK_ASSERT((backBlock->getHeight() + 1) == block.getHeight() &&
               "cache corruption: appended block should have height equal to "
               "previous block height + 1");
  }

  const auto existing = cache_.find(&block);
  if (existing != cache_.end()) {
    return existing->second;
  }

  auto score = scoreFromEndorsements(block);

  history_.push_back(&block);
  cache_.insert({&block, score});

  const index_t* newFrontBlock = history_.front();

  // erase from the map if the ring buffer is full
  if (frontBlock != nullptr && newFrontBlock != frontBlock) {
    auto it = cache_.find(frontBlock);
    VBK_ASSERT(it != cache_.end() &&
               "cache corruption: expected to find block in cache");
    cache_.erase(it);
  }

  return score;
}

PopPayouts DefaultPopRewardsCalculator::calculatePayouts(
    const BlockIndex<AltBlock>& endorsedBlock) {
  // make sure cache is in valid state, eg contains all necessary
  // blocks to calculate POP difficulty for the endorsed block

  size_t toFetch =
      tree_.getParams().getPayoutParams().difficultyAveragingInterval();
  if ((int)toFetch > endorsedBlock.getHeight()) {
    toFetch = endorsedBlock.getHeight();
  }
  auto* historyLast = history_.empty() ? nullptr : history_.back();
  size_t historySize = history_.size();
  auto missingBlocks =
      fetchBlocksUntil(endorsedBlock.pprev, historyLast, toFetch);

  bool beginOk = true;
  bool endOk = true;

  if (history_.empty()) {
    beginOk = false;
  }

  if (beginOk && endOk &&
      (missingBlocks.size() > 0 &&
       missingBlocks.back()->pprev != history_.back())) {
    endOk = false;
  }

  if (beginOk && endOk) {
    // now make a check that first difficulty block exists in the cache
    const auto* beginBlock =
        endorsedBlock.getAncestorBlocksBehind((int)toFetch);
    size_t historyBeginOffset = toFetch - missingBlocks.size();
    if (historySize < historyBeginOffset ||
        history_[historySize - historyBeginOffset] != beginBlock) {
      beginOk = false;
    }
  }

  if (!beginOk || !endOk) {
    invalidateCache();
  }
  historyLast = history_.empty() ? nullptr : history_.back();

  auto difficultyBlocks =
      fetchBlocksUntil(endorsedBlock.pprev, historyLast, toFetch);

  for (const auto& b : reverse_iterate(difficultyBlocks)) {
    appendToCache(*b);
  }

  auto blockScore = appendToCache(endorsedBlock);
  auto popDifficulty = calculateDifficulty(endorsedBlock);
  return calculatePayoutsInner(endorsedBlock, blockScore, popDifficulty);
}

void DefaultPopRewardsCalculator::invalidateCache() {
  cache_.clear();
  history_.clear();
}

void DefaultPopRewardsCalculator::eraseCacheHistory(uint32_t blocks) {
  for (uint32_t i = 0; i < blocks; i++) {
    if (history_.empty()) break;
    const auto* block = history_.pop_back();
    auto it = cache_.find(block);
    if (it == cache_.end()) continue;
    cache_.erase(it);
  }
}

void DefaultPopRewardsCalculator::onOverrideTip(const index_t& index) {
  // invalidate rewards cache if necessary
  uint32_t invalidBlocks = std::numeric_limits<uint32_t>::max();

  const auto* fork = tree_.getBestChain().findFork(&index);
  if (fork != nullptr) {
    auto* tip = tree_.getBestChain().tip();
    VBK_ASSERT(tip);
    invalidBlocks = tip->getHeight() - fork->getHeight();
  }
  eraseCacheHistory(invalidBlocks);
}

PopPayouts DefaultPopRewardsCalculator::getPopPayout(
    const AltBlockTree::hash_t& tip) {
  VBK_ASSERT(tree_.isBootstrapped() && "not bootstrapped");

  auto* index = tree_.getBlockIndex(tip);
  VBK_ASSERT_MSG(index, "can not find block %s", HexStr(tip));
  VBK_ASSERT_MSG(index == tree_.getBestChain().tip(),
                 "AltTree is at unexpected state: Tip=%s ExpectedTip=%s",
                 tree_.getBestChain().tip()->toPrettyString(),
                 index->toPrettyString());

  VBK_ASSERT_MSG(index->isValidUpTo(BLOCK_CONNECTED),
                 "Block %s is not connected",
                 index->toPrettyString());

  // -1 because we want to pay for a block E in block E+PD,
  // where E is endorsed height, PD is payout delay.
  auto* endorsedBlock = index->getAncestorBlocksBehind(
      tree_.getParams().getPayoutParams().getPopPayoutDelay() - 1);
  if (endorsedBlock == nullptr) {
    // not enough blocks for payout
    return {};
  }

  VBK_ASSERT_MSG(index->getHeight() >=
                     (endorsedBlock->getHeight() +
                      tree_.getParams().getEndorsementSettlementInterval() - 1),
                 "Block %s is not finalized for PoP payouts",
                 endorsedBlock->toPrettyString());

  auto ret = calculatePayouts(*endorsedBlock);
  VBK_LOG_DEBUG("Block %s, paying to %d addresses",
                index->toShortPrettyString(),
                ret.size());
  return ret;
}

}  // namespace altintegration
