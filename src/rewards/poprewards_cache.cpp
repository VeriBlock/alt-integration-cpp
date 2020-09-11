// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/rewards/poprewards_cache.hpp>

namespace altintegration {

PopRewardsBigDecimal PopRewardsCache::calculateDifficulty(
    const index_t& tip) const {
  auto it = buffer.crbegin();
  VBK_ASSERT(!buffer.empty() && "cache corruption: expected not empty");
  VBK_ASSERT((it->first == &tip) && "cache corruption: expected tip as last item");

  // remove last element - it is the current block that we calculate difficulty
  // for
  ++it;
  PopRewardsBigDecimal sumscore = 0.0;
  size_t count = 0;
  uint64_t difficultyInterval =
      altParams_->getRewardParams().difficultyAveragingInterval();
  for (; it != buffer.crend(); ++it) {
    if (count >= difficultyInterval) {
      break;
    }
    count++;
    sumscore += it->second;
  }

  VBK_ASSERT_MSG((tip.getHeight() <= (int64_t)(difficultyInterval + 1)) ||
                     (count >= difficultyInterval),
                 "cache corruption: expected at least %llu blocks",
                 difficultyInterval);

  auto difficulty = sumscore / difficultyInterval;

  // Minimum difficulty
  if (difficulty < 1.0) {
    difficulty = 1.0;
  }
  return difficulty;
}

std::map<std::vector<uint8_t>, int64_t> PopRewardsCache::calculatePayouts(
    const index_t& endorsedBlock) {
  auto popDifficulty = updateAndCalculateDifficulty(endorsedBlock);
  pair_t cached;
  bool ret = findCached(endorsedBlock, cached);
  VBK_ASSERT(ret && "block should be in cache");
  return PopRewards::calculatePayoutsInner(
      endorsedBlock, cached.second, popDifficulty);
}

PopRewardsBigDecimal PopRewardsCache::updateAndCalculateDifficulty(
    const index_t& endorsed) {
  updateCache(endorsed);
  return calculateDifficulty(endorsed);
}

bool PopRewardsCache::findCached(const index_t& block, pair_t& out) {
  if (buffer.empty()) return false;
  VBK_ASSERT(buffer.back().first != nullptr && "cache corruption: expected valid back");
  VBK_ASSERT(buffer.front().first != nullptr && "cache corruption: expected valid front");
  auto lastHeight = buffer.back().first->getHeight();
  auto firstHeight = buffer.front().first->getHeight();
  if (block.getHeight() > lastHeight) return false;
  if (block.getHeight() < firstHeight) return false;

  const auto& cached = buffer[block.getHeight() - firstHeight];
  if (cached.first != &block) return false;

  out = cached;
  return true;
}

bool PopRewardsCache::findCached(const index_t& block) {
  pair_t out;
  bool ret = findCached(block, out);
  (void)out;
  return ret;
}

void PopRewardsCache::appendToCache(const index_t& block) {
  auto score = scoreFromEndorsements(block);
  buffer.push_back({&block, score});
}

void PopRewardsCache::truncateCacheHigherThan(const index_t& fromBlock) {
  while (!buffer.empty() && buffer.back().first != &fromBlock) {
    buffer.pop_back();
  }
}

void PopRewardsCache::updateCache(const index_t& endorsed) {
  auto* begin = endorsed.getAncestorBlocksBehind(
      altParams_->getRewardParams().difficultyAveragingInterval());
  auto* end = endorsed.pprev;

  bool rangeExists = (begin != nullptr) && (end != nullptr) &&
                     findCached(*begin) && findCached(*end);
  if (!rangeExists) {
    rebuildCache(endorsed);
    return;
  }

  if (findCached(endorsed)) {
    truncateCacheHigherThan(endorsed);
    return;
  }

  truncateCacheHigherThan(*end);
  appendToCache(endorsed);
}

void PopRewardsCache::rebuildCache(const index_t& endorsed) {
  size_t toFetch =
      altParams_->getRewardParams().difficultyAveragingInterval() + 1;
  buffer.clear();
  std::vector<const index_t*> blocks;
  blocks.reserve(toFetch);
  auto* cur = &endorsed;
  for (size_t i = 0; i < toFetch; i++) {
    if (cur == nullptr) break;
    blocks.push_back(cur);
    cur = cur->pprev;
  }

  for (const auto& b : reverse_iterate(blocks)) {
    appendToCache(*b);
  }
}

}  // namespace altintegration
