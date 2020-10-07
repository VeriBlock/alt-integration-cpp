// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_HPP_

#include <memory>
#include <random>
#include "miner_worker.hpp"

namespace altintegration {

//! @private
//! dumb POW miner used in tests
template <typename Block, typename ChainParams>
struct Miner {
  using merkle_t = typename Block::merkle_t;
  using index_t = BlockIndex<Block>;
  using worker_t = MinerWorker<Block, ChainParams>;
  using range_t = typename worker_t::range_t;
  using nonce_t = typename worker_t::nonce_t;

  Miner(const ChainParams& params, size_t threads = 0)
      : params_(params) {
    // prepare signals for workers
    finishedSignal_ = std::make_shared<std::condition_variable>();
    finishedLock_ = std::make_shared<std::mutex>();

    // try to detect concurrent threads count
    if (threads == 0) {
      threads = std::thread::hardware_concurrency();
    }
    // make sure we have at least one worker thread
    if (threads == 0) {
      threads = 1;
    }

    workers_.reserve(threads);
    auto ranges = getRanges(threads);

    for (size_t i = 0; i < threads; i++) {
      auto worker = std::make_shared<worker_t>(
          params_, ranges[i], *finishedSignal_, *finishedLock_);
      workers_.push_back(worker);
    }
  }

  void createBlock(Block& block) {
    {
      std::unique_lock<std::mutex> guard(*finishedLock_);
      for (const auto& w : workers_) {
        w->start(block);
      }
      finishedSignal_->wait(guard);
    }
    
    // stop all workers to avoid useless computing cycles
    for (const auto& w : workers_) {
      w->stop();
    }

    bool blockReady = false;
    // we wait for all threads to finish but save only first result
    for (const auto& w : workers_) {
      auto candidate = w->waitResult();
      if (candidate.first == false) continue;
      if (blockReady) continue;
      block = candidate.second;
      blockReady = true;
    }
  }

  // One must define their own template specialization for given Block and
  // ChainParams types. Otherwise, get pretty compilation error.
  Block getBlockTemplate(const BlockIndex<Block>& tip,
                         const merkle_t& merkleRoot);

  Block createNextBlock(const index_t& prev, const merkle_t& merkle) {
    Block block = getBlockTemplate(prev, merkle);
    createBlock(block);
    return block;
  }

  Block createNextBlock(const index_t& prev) {
    merkle_t merkle;
    std::generate(
        merkle.begin(), merkle.end(), []() { return uint8_t(rand() & 0xff); });
    return createNextBlock(prev, merkle);
  }

 private:
  const ChainParams& params_;
  std::vector<std::shared_ptr<worker_t>> workers_;
  std::shared_ptr<std::condition_variable> finishedSignal_;
  std::shared_ptr<std::mutex> finishedLock_;

  std::vector<range_t> getRanges(size_t threads) {
    assert(threads > 0);
    nonce_t workerRangeSize =
        (nonce_t)(std::numeric_limits<nonce_t>::max() / threads);
    nonce_t workerRangeLeftovers = std::numeric_limits<nonce_t>::max() -
                                   (nonce_t)(workerRangeSize * threads);
    assert(workerRangeSize > 0);

    std::vector<range_t> ranges;
    ranges.reserve(threads);
    nonce_t begin = 0;
    for (size_t i = 0; i < threads; i++) {
      nonce_t end = begin + workerRangeSize - 1;
      if (workerRangeLeftovers > 0) {
        end++;
        workerRangeLeftovers--;
      }
      auto range = range_t(begin, end);
      ranges.push_back(range);
      begin = end + 1;
    }
    return ranges;
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_HPP_
