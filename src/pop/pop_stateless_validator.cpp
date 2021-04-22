// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/pop_stateless_validator.hpp>
#include <veriblock/pop/stateless_validation.hpp>

namespace altintegration {

static unsigned long upper_power_of_two(uint32_t v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

// hack to build std::future if VBK_NO_THREADS is defined
template <typename T>
std::future<T> make_future(T&& t) {
  std::packaged_task<T()> task([t]() { return t; });
  auto future = task.get_future();
  task();
  return future;
}

PopValidator::PopValidator(const VbkChainParams& vbk,
                           const BtcChainParams& btc,
                           const AltChainParams& alt,
                           size_t threads)
    : threads_(threads), vbk_(vbk), btc_(btc), alt_(alt) {
  start(threads);
}

void PopValidator::start(size_t threads) {
  threads_ = threads;

#ifndef VBK_NO_THREADS
  VBK_ASSERT_MSG(workers == nullptr, "PopValidator has already been started");

  // try to detect concurrent threads count
  if (threads == 0) {
    threads = std::thread::hardware_concurrency();
  }
  // make sure we have at least one worker thread
  if (threads == 0) {
    threads = 1;
  }
  tp::ThreadPoolOptions options;
  options.setThreadCount(threads);
  // queue size can "at least" contain full PopData
  auto size = upper_power_of_two((uint32_t)alt_.maxWorkerQueueSize());
  options.setQueueSize(size);
  workers = std::make_shared<ValidationThreadPool>(options);
#endif
}

void PopValidator::stop() {
#ifndef VBK_NO_THREADS
  workers = nullptr;
#endif
}

PopValidator::~PopValidator() { stop(); }

void PopValidator::clear() {
  // TODO: clear validation queue
}

template <>
std::future<ValidationState> PopValidator::addCheck(const VbkBlock& b) {
#ifndef VBK_NO_THREADS
  VBK_ASSERT_MSG(workers != nullptr, "PopValidator is stopped");

  std::packaged_task<ValidationState()> t([&]() -> ValidationState {
    ValidationState state;
    checkBlock(b, state, vbk_);
    return state;
  });
  std::future<ValidationState> r = t.get_future();
  bool success = workers->tryPost(t);
  VBK_ASSERT_MSG(success,
                 "Worker queue is full, can't add new item. Max size=%d",
                 upper_power_of_two((uint32_t)alt_.maxWorkerQueueSize()));
  return r;
#else
  ValidationState state;
  checkBlock(b, state, vbk_);
  return make_future<ValidationState>(std::move(state));
#endif
}

template <>
std::future<ValidationState> PopValidator::addCheck(const VTB& b) {
#ifndef VBK_NO_THREADS
  VBK_ASSERT_MSG(workers != nullptr, "PopValidator is stopped");

  std::packaged_task<ValidationState()> t([&]() {
    ValidationState state;
    checkVTB(b, state, btc_, vbk_);
    return state;
  });
  std::future<ValidationState> r = t.get_future();
  bool success = workers->tryPost(t);
  VBK_ASSERT_MSG(success,
                 "Worker queue is full, can't add new item. Max size=%d",
                 upper_power_of_two((uint32_t)alt_.maxWorkerQueueSize()));
  return r;
#else
  ValidationState state;
  checkVTB(b, state, btc_, vbk_);
  return make_future<ValidationState>(std::move(state));
#endif
}

template <>
std::future<ValidationState> PopValidator::addCheck(const ATV& b) {
#ifndef VBK_NO_THREADS
  VBK_ASSERT_MSG(workers != nullptr, "PopValidator is stopped");

  std::packaged_task<ValidationState()> t([&]() {
    ValidationState state;
    checkATV(b, state, alt_, vbk_);
    return state;
  });
  std::future<ValidationState> r = t.get_future();
  bool success = workers->tryPost(t);
  VBK_ASSERT_MSG(success,
                 "Worker queue is full, can't add new item. Max size=%d",
                 upper_power_of_two((uint32_t)alt_.maxWorkerQueueSize()));
  return r;
#else
  ValidationState state;
  checkATV(b, state, alt_, vbk_);
  return make_future<ValidationState>(std::move(state));
#endif
}

}  // namespace altintegration
