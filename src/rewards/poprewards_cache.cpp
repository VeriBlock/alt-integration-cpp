// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/rewards/poprewards_cache.hpp>

namespace altintegration {

PopRewardsBigDecimal PopRewardsCache::scoreFromEndorsements(
    const PopRewardsCalculatorInterface& calculator,
    const BlockIndex<AltBlock>& endorsedBlock) {
  const auto it = cache_.find(&endorsedBlock);
  if (it == cache_.end()) {
    return PopRewards::scoreFromEndorsements(calculator, endorsedBlock);
  }
  return it->second;
}

static std::vector<const PopRewardsCache::index_t*> fetchBlocksUntil(
    const PopRewardsCache::index_t* from,
    const PopRewardsCache::index_t* check,
    size_t count) {
  std::vector<const PopRewardsCache::index_t*> blocks;
  blocks.reserve(count);
  const PopRewardsCache::index_t* curBlock = from;

  for (size_t i = 0; i < count; i++) {
    if (curBlock == nullptr) break;
    if (curBlock == check) break;
    blocks.push_back(curBlock);
    curBlock = curBlock->pprev;
  }
  return blocks;
}

std::map<std::vector<uint8_t>, int64_t> PopRewardsCache::calculatePayouts(
    const PopRewardsCalculatorInterface& calculator,
    const index_t& endorsedBlock) {
  // make sure cache is in valid state, eg contains all necessary
  // blocks to calculate POP difficulty for the endorsed block

  size_t toFetch = altParams_->getPayoutParams().difficultyAveragingInterval();
  if ((int)toFetch > endorsedBlock.getHeight()) {
    toFetch = endorsedBlock.getHeight();
  }
  auto* historyLast = history_.empty() ? nullptr : history_.back();
  size_t historySize = history_.size();
  auto missingBlocks = fetchBlocksUntil(endorsedBlock.pprev, historyLast, toFetch);

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
    appendToCache(calculator, *b);
  }

  auto blockScore = appendToCache(calculator, endorsedBlock);
  auto popDifficulty = calculateDifficulty(calculator, endorsedBlock);
  return calculatePayoutsInner(
      calculator, endorsedBlock, blockScore, popDifficulty);
}

void PopRewardsCache::invalidateCache() {
  cache_.clear();
  history_.clear();
}

void PopRewardsCache::eraseCacheHistory(uint32_t blocks) {
  for (uint32_t i = 0; i < blocks; i++) {
    if (history_.empty()) break;
    const auto* block = history_.pop_back();
    auto it = cache_.find(block);
    if (it == cache_.end()) continue;
    cache_.erase(it);
  }
}

PopRewardsBigDecimal PopRewardsCache::appendToCache(
    const PopRewardsCalculatorInterface& calculator, const index_t& block) {
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

  auto score = PopRewards::scoreFromEndorsements(calculator, block);

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

}  // namespace altintegration
