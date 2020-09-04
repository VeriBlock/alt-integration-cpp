// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CACHE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CACHE_HPP_

#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/rewards/poprewards.hpp>

namespace altintegration {

template <class T>
struct circular_buffer {
  explicit circular_buffer(size_t size)
      : buf_(std::unique_ptr<T[]>(new T[size])), max_size_(size) {}

  void put(T item) {
    buf_[tail_] = item;
    tail_ = (tail_ + 1) % max_size_;
    if (head_ == tail_) {
      head_ = (head_ + 1) % max_size_;
    }
  }

  bool get(const size_t pos, T& out) {
    out = buf_[pos % max_size_];
    return true;
  }

  void eraseUpTo(const size_t pos) {
    size_t inpos = pos % max_size_;
    if (head_ < tail_ && inpos < head_) return;
    if (head_ < tail_ && inpos > tail_) return;
    if (head_ > tail_ && inpos > tail_ && inpos < head_) return;
    tail_ = pos % max_size_;
  }

  int find(const std::function<bool(const T&)>& pred) {
    size_t cur = head_;
    for (size_t i = 0; i < max_size_; i++) {
      if (cur == tail_) break;
      if (pred(buf_[cur])) return (int)cur;
      cur = (cur + 1) % max_size_;
    }
    return -1;
  }

  std::vector<T> asVector() const {
    std::vector<T> out;
    size_t cur = head_;
    for (size_t i = 0; i < max_size_; i++) {
      if (cur == tail_) break;
      out.push_back(buf_[cur]);
      cur = (cur + 1) % max_size_;
    }
    return out;
  }

  void reset() {
    head_ = 0;
    tail_ = 0;
  }

  private:
  std::unique_ptr<T[]> buf_;
  size_t head_ = 0;
  size_t tail_ = 0;
  const size_t max_size_;
};

/**
 * @invariant does not modify any on-disk state.
 */
struct PopRewardsCache : public PopRewards {
  using index_t = BlockIndex<AltBlock>;
  using pair_t = std::pair<index_t, PopRewardsBigDecimal>;

  PopRewardsCache(const AltChainParams& altParams, const VbkBlockTree& vbk_tree)
      : PopRewards(altParams, vbk_tree),
        altParams_(&altParams),
        vbkTree_(&vbk_tree),
        buffer(altParams.getRewardParams().difficultyAveragingInterval() * 2) {}

  virtual ~PopRewardsCache() = default;

  virtual PopRewardsBigDecimal updateAndCalculateDifficulty(const index_t& tip);

  PopRewardsBigDecimal calculateDifficulty(
      const BlockIndex<AltBlock>& tip) const override;

  std::map<std::vector<uint8_t>, int64_t> calculatePayouts(
      const index_t& endorsedBlock,
      const PopRewardsBigDecimal& popDifficulty) override;

 protected:
  const AltChainParams* altParams_;
  const VbkBlockTree* vbkTree_;
  circular_buffer<pair_t> buffer;

  virtual int find(const index_t& block) {
    auto pos = buffer.find([&block](const pair_t& item) {
      return item.first.getHash() == block.getHash();
    });
    return pos;
  }

  virtual bool cutend(const index_t& block) {
    auto pos = find(block);
    if (pos < 0) {
      return false;
    }
    buffer.eraseUpTo(pos);
    return true;
  }

  virtual void appendEndorsed(const index_t& block);

  virtual void updateCached(const index_t& endorsed);

  virtual void invalidateCache(const index_t& endorsed);
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPREWARDS_CACHE_HPP_
