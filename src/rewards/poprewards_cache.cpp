// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/rewards/poprewards_cache.hpp>

namespace altintegration {

PopRewardsBigDecimal PopRewardsCache::scoreFromEndorsements(
    const BlockIndex<AltBlock>& endorsedBlock) {
  const auto it = cache_.find(&endorsedBlock);
  if (it == cache_.end()) {
    return PopRewards::scoreFromEndorsements(endorsedBlock);
  }
  return it->second;
}

std::map<std::vector<uint8_t>, int64_t> PopRewardsCache::calculatePayouts(
    const index_t& endorsedBlock) {
  // make sure cache is in valid state, eg contains all necessary
  // blocks to calculate POP difficulty for the endorsed block

  const auto* curBlock = endorsedBlock.pprev;
  size_t toFetch = altParams_->getRewardParams().difficultyAveragingInterval();
  if (toFetch > endorsedBlock.getHeight()) {
    toFetch = endorsedBlock.getHeight();
  }
  std::vector<const index_t*> difficultyBlocks;
  difficultyBlocks.reserve(toFetch);

  for (size_t i = 0; i < toFetch; i++) {
    if (curBlock == nullptr) break;
    if (!history_.empty() && history_.back() == curBlock) break;
    difficultyBlocks.push_back(curBlock);
    curBlock = curBlock->pprev;
  }

  // now make a check that first difficulty block exists in the cache
  const auto* beginBlock = endorsedBlock.getAncestorBlocksBehind((int)toFetch);
  toFetch -= difficultyBlocks.size();
  size_t historySize = history_.size();
  bool beginOk = true;
  bool endOk = true;

  if (history_.empty() || historySize < toFetch ||
      history_[historySize - toFetch] != beginBlock) {
    beginOk = false;
  }

  if (beginOk && endOk &&
      (difficultyBlocks.size() > 0 && difficultyBlocks.back() == nullptr)) {
    endOk = false;
  }

  if (beginOk && endOk &&
      (difficultyBlocks.size() > 0 &&
       difficultyBlocks.back()->pprev != history_.back())) {
    endOk = false;
  }

  if (!beginOk || !endOk) {
    invalidateCache();
  }

  for (const auto& b : reverse_iterate(difficultyBlocks)) {
    appendToCache(*b);
  }

  auto blockScore = appendToCache(endorsedBlock);
  auto popDifficulty = calculateDifficulty(endorsedBlock);
  return calculatePayoutsInner(endorsedBlock, blockScore, popDifficulty);
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

PopRewardsBigDecimal PopRewardsCache::appendToCache(const index_t& block) {
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

  auto score = PopRewards::scoreFromEndorsements(block);

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
