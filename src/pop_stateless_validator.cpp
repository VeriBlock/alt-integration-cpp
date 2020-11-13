// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop_stateless_validator.hpp>
#include <veriblock/stateless_validation.hpp>

namespace altintegration {

PopValidator::PopValidator(const VbkChainParams& vbk,
                           const BtcChainParams& btc,
                           const AltChainParams& alt,
                           size_t threads)
    : vbk_(vbk), btc_(btc), alt_(alt), threads_(threads) {
  start();
}

void PopValidator::start() {
  VBK_ASSERT(workers == nullptr && "PopValidator has already been started");

  // try to detect concurrent threads count
  if (threads_ == 0) {
    threads_ = std::thread::hardware_concurrency();
  }
  // make sure we have at least one worker thread
  if (threads_ == 0) {
    threads_ = 1;
  }
  workers = std::make_shared<ThreadPool>(threads_);
}

void PopValidator::stop() {
  workers = nullptr;
}

template <>
std::future<ValidationState> PopValidator::addCheck(const VbkBlock& block) {
  return workers->enqueue([&] {
    ValidationState state;
    checkBlock(block, state, vbk_);
    return state;
  });
}

template <>
std::future<ValidationState> PopValidator::addCheck(const VTB& vtb) {
  return workers->enqueue([&] {
    ValidationState state;
    checkVTB(vtb, state, btc_);
    return state;
  });
}

template <>
std::future<ValidationState> PopValidator::addCheck(const ATV& atv) {
  return workers->enqueue([&] {
    ValidationState state;
    checkATV(atv, state, alt_);
    return state;
  });
}

}  // namespace altintegration
