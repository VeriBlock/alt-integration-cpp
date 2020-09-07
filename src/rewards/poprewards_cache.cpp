// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/rewards/poprewards_cache.hpp>

namespace altintegration {

PopRewardsBigDecimal PopRewardsCache::calculateDifficulty(
    const index_t& /* ignore */) const {
  auto it = buffer.crbegin();
  // remove last element - it is the current block that we calculate difficulty
  // for
  ++it;
  PopRewardsBigDecimal sumscore = 0.0;
  size_t count = 0;
  for (; it != buffer.crend(); ++it) {
    if (count >= altParams_->getRewardParams().difficultyAveragingInterval()) {
      break;
    }
    count++;
    sumscore += it->second;
  }

  auto difficulty =
      sumscore /
      static_cast<uint64_t>(
          altParams_->getRewardParams().difficultyAveragingInterval());

  // Minimum difficulty
  if (difficulty < 1.0) {
    difficulty = 1.0;
  }
  return difficulty;
}

std::map<std::vector<uint8_t>, int64_t> PopRewardsCache::calculatePayouts(
    const index_t& endorsedBlock) {
  auto popDifficulty = updateAndCalculateDifficulty(endorsedBlock);
  auto it = std::find_if(
      buffer.rbegin(), buffer.rend(), [&endorsedBlock](const pair_t& item) {
        return item.first.getHash() == endorsedBlock.getHash();
      });
  VBK_ASSERT(it != buffer.rend() && "block should be in cache");
  return PopRewards::calculatePayoutsInner(
      endorsedBlock, it->second, popDifficulty);
}

PopRewardsBigDecimal PopRewardsCache::updateAndCalculateDifficulty(
    const index_t& endorsed) {
  cacheRebuild(endorsed);
  return calculateDifficulty(endorsed);
}

void PopRewardsCache::cacheAddBlock(const index_t& block) {
  auto score = scoreFromEndorsements(block);
  buffer.push_back({block, score});
}

bool PopRewardsCache::cacheBlockExists(const index_t* block) {
  if (block == nullptr) {
    return false;
  }
  auto it =
      std::find_if(buffer.rbegin(), buffer.rend(), [block](const pair_t& item) {
        return item.first.getHash() == block->getHash();
      });
  if (it == buffer.rend()) {
    return false;
  }
  return true;
}

void PopRewardsCache::cacheInvalidate(const index_t& endorsed) {
  buffer.clear();
  std::vector<index_t> blocks;
  auto* cur = &endorsed;
  for (size_t i = 0;
       i < altParams_->getRewardParams().difficultyAveragingInterval();
       i++) {
    if (cur == nullptr) break;
    blocks.push_back(*cur);
    cur = cur->pprev;
  }

  for (const auto& b : reverse_iterate(blocks)) {
    cacheAddBlock(b);
  }
}

void PopRewardsCache::cacheTruncateTailTo(const index_t& lastBlock) {
  auto it = buffer.rbegin();
  while (true) {
    if (buffer.empty()) break;
    if (it == buffer.rend()) break;
    if (it->first.getHash() == lastBlock.getHash()) return;
    buffer.pop_back();
    it = buffer.rbegin();
  }
  return;
}

void PopRewardsCache::cacheRebuild(const index_t& endorsed) {
  auto* begin = endorsed.getAncestorBlocksBehind(
      altParams_->getRewardParams().difficultyAveragingInterval() + 1);
  auto* end = endorsed.pprev;

  bool rangeExists = true;
  rangeExists = rangeExists && cacheBlockExists(begin);
  rangeExists = rangeExists && cacheBlockExists(end);

  if (!rangeExists) {
    cacheInvalidate(endorsed);
    return;
  }

  if (cacheBlockExists(&endorsed)) {
    cacheTruncateTailTo(endorsed);
    return;
  }

  cacheTruncateTailTo(*end);
  cacheAddBlock(endorsed);
}

}  // namespace altintegration
