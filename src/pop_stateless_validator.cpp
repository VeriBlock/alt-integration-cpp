// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop_stateless_validator.hpp>
#include <veriblock/stateless_validation.hpp>

#ifndef VBK_NO_THREADS
#include <veriblock/third_party/ThreadPool.h>
#endif

namespace altintegration {

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
    : vbk_(vbk), btc_(btc), alt_(alt) {
  start(threads);
}

void PopValidator::start(size_t threads) {
  (void)threads;

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
  workers = std::make_shared<third_party::ThreadPool>(threads);
#endif
}

void PopValidator::stop() {
#ifndef VBK_NO_THREADS
  workers = nullptr;
#endif
}

PopValidator::~PopValidator() { stop(); }

template <>
std::future<ValidationState> PopValidator::addCheck(const VbkBlock& block) {
#ifndef VBK_NO_THREADS
  VBK_ASSERT_MSG(workers != nullptr, "PopValidator is stopped");
  return workers->enqueue([&] {
    ValidationState state;
    checkBlock(block, state, vbk_);
    return state;
  });
#else
  ValidationState state;
  checkBlock(block, state, vbk_);
  return make_future<ValidationState>(std::move(state));
#endif
}

template <>
std::future<ValidationState> PopValidator::addCheck(const VTB& vtb) {
#ifndef VBK_NO_THREADS
  VBK_ASSERT_MSG(workers != nullptr, "PopValidator is stopped");
  return workers->enqueue([&] {
    ValidationState state;
    checkVTB(vtb, state, btc_);
    return state;
  });
#else
  ValidationState state;
  checkVTB(vtb, state, btc_);
  return make_future<ValidationState>(std::move(state));
#endif
}

template <>
std::future<ValidationState> PopValidator::addCheck(const ATV& atv) {
#ifndef VBK_NO_THREADS
  VBK_ASSERT_MSG(workers != nullptr, "PopValidator is stopped");
  return workers->enqueue([&] {
    ValidationState state;
    checkATV(atv, state, alt_);
    return state;
  });
#else
  ValidationState state;
  checkATV(atv, state, alt_);
  return make_future<ValidationState>(std::move(state));
#endif
}

}  // namespace altintegration
