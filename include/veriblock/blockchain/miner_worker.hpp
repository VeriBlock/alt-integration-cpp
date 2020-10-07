// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_WORKER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_WORKER_HPP_

#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>
#include <veriblock/stateless_validation.hpp>

namespace altintegration {

template <typename Block, typename ChainParams>
struct MinerWorker {
  using nonce_t = typename Block::nonce_t;
  using range_t = std::pair<nonce_t, nonce_t>;

  /**
   * Create worker.
   * @param params chain params to generate proof of work with
   * @param nonce_range pair with start and end range for nonce generation
   * @param blockTemplate use this block template for new block generation
   * @param finished_signal this condition variable will be triggered when block
   * is mined
   * @param finished_lock this mutex protects finished_signal from concurrent
   * modification
   */
  MinerWorker(const ChainParams& params,
              const range_t& nonce_range,
              std::condition_variable& finished_signal,
              std::mutex& finished_lock)
      : params_(params),
        nonce_range_(nonce_range),
        finished_(finished_signal),
        finished_lock_(finished_lock) {
    VBK_ASSERT(nonce_range.first <= nonce_range.second &&
               "invalid nonce range");
  }

  /**
   * Ask worker to start running.
   */
  void start(const Block& block_template) {
    assert(!runner.joinable() && "thread already running");
    block_ = block_template;
    block_.setNonce(nonce_range_.first);
    terminated_ = false;
    block_ready_ = false;
    runner = std::thread(&MinerWorker<Block, ChainParams>::run, this);
  }

  /**
   * Ask worker to stop running.
   */
  void stop() { terminated_ = true; }

  /**
   * Should be called in the end of the life cycle of each worker.
   * Returns a mined block with 'true' status of some temporary data with
   * 'false' status.
   * @return pair<status, block>. When status is false block contents should be
   * ignored.
   */
  std::pair<bool, Block&> waitResult() {
    if (runner.joinable()) {
      runner.join();
    }
    return {block_ready_, block_};
  }

 protected:
  void run() {
    while (true) {
      if (terminated_) return;
      if (createBlockStep()) {
        {
          std::lock_guard<std::mutex> guard(finished_lock_);
          block_ready_ = true;
          terminated_ = true;
        }
        finished_.notify_one();
      }
    }
  }

  bool createBlockStep() {
    bool generated = checkProofOfWork(block_, params_);
    if (generated) return true;

    auto nonce = block_.getNonce();
    if (nonce > nonce_range_.second ||
        nonce >= std::numeric_limits<nonce_t>::max()) {
      block_.setTimestamp(block_.getTimestamp() + 1);
      nonce = nonce_range_.first;
    } else {
      nonce++;
    }
    block_.setNonce(nonce);
    return false;
  }

 private:
  // mining
  const ChainParams& params_;
  const range_t nonce_range_;
  Block block_;

  // thread management
  std::thread runner;
  std::atomic_bool terminated_{false};
  std::atomic_bool block_ready_{false};
  std::condition_variable& finished_;
  std::mutex& finished_lock_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_WORKER_HPP_
