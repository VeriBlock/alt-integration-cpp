// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/rewards/poprewards.hpp>
#include <veriblock/rewards/poprewards_cache.hpp>

namespace altintegration {

PopRewardsBigDecimal PopRewardsCache::calculateDifficulty(
    const BlockIndex<AltBlock>& /* ignore */) const {
  auto cached = buffer.asVector();
  // remove last element - it is the current block that we calculate difficulty for
  cached.pop_back();
  PopRewardsBigDecimal sumscore = 0.0;
  size_t count = 0;
  for (const auto& c : reverse_iterate(cached)) {
    if (count >= altParams_->getRewardParams().difficultyAveragingInterval()) {
      break;
    }
    count++;
    sumscore += c.second;
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

PopRewardsBigDecimal PopRewardsCache::updateAndCalculateDifficulty(
    const index_t& endorsed) {
  updateCached(endorsed);
  return calculateDifficulty(endorsed);
}

std::map<std::vector<uint8_t>, int64_t> PopRewardsCache::calculatePayouts(
    const index_t& endorsedBlock, const PopRewardsBigDecimal& popDifficulty) {
  auto pos = find(endorsedBlock);
  VBK_ASSERT(pos >= 0 &&
             "Block should be in cache. updateAndCalculateDifficulty() should "
             "be called to obtain POP difficulty.");
  pair_t cached;
  bool found = buffer.get(pos, cached);
  VBK_ASSERT(found &&
             "Block should be in cache. updateAndCalculateDifficulty() should "
             "be called to obtain POP difficulty.");
  return PopRewards::calculatePayouts(
      endorsedBlock, cached.second, popDifficulty);
}

void PopRewardsCache::updateCached(const index_t& endorsed) {
  auto* begin = endorsed.getAncestorBlocksBehind(
      altParams_->getRewardParams().difficultyAveragingInterval() + 1);
  auto* end = endorsed.pprev;
  bool existsEndorsed = (find(endorsed) >= 0);
  bool existsBegin = (begin == nullptr) ? false : (find(*begin) >= 0);
  bool existsEnd = (end == nullptr) ? false : (find(*end) >= 0);

  if (!existsEnd || !existsBegin) {
    invalidateCache(endorsed);
    return;
  }

  if (!existsEndorsed) {
    cutend(*end);
  }
  appendEndorsed(endorsed);
}

void PopRewardsCache::appendEndorsed(const index_t& block) {
  auto score = scoreFromEndorsements(block);
  buffer.put({block, score});
}

void PopRewardsCache::invalidateCache(const index_t& endorsed) {
  buffer.reset();
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
    appendEndorsed(b);
  }
}

}  // namespace altintegration
