// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/stateless_validation.hpp>
#include <veriblock/pop_stateless_validator.hpp>

namespace altintegration {

ValidationState PopCheckBlock::operator()() {
  ValidationState state;
  checkBlock(block_, state, vbk_);
  return state;
}

ValidationState PopCheckVtb::operator()() {
  ValidationState state;
  checkVTB(vtb_, state, btc_);
  return state;
}

ValidationState PopCheckAtv::operator()() {
  ValidationState state;
  checkATV(atv_, state, alt_);
  return state;
}

PopValidator::PopValidator(const VbkChainParams& vbk,
                           const BtcChainParams& btc,
                           const AltChainParams& alt,
                           size_t threads)
    : vbk_(vbk), btc_(btc), alt_(alt) {
  // try to detect concurrent threads count
  if (threads == 0) {
    threads = std::thread::hardware_concurrency();
  }
  // make sure we have at least one worker thread
  if (threads == 0) {
    threads = 1;
  }

  workers = std::make_shared<ThreadPool>(threads);
}

template <>
std::future<ValidationState> PopValidator::addCheck(const VbkBlock& block)
{
  return workers->enqueue([&] {
    PopCheckBlock blockCheck(vbk_, block);
    return blockCheck();
  });
}

template <>
std::future<ValidationState> PopValidator::addCheck(const VTB& vtb) {
  return workers->enqueue([&] {
    PopCheckVtb vtbCheck(btc_, vtb);
    return vtbCheck();
  });
}

template <>
std::future<ValidationState> PopValidator::addCheck(const ATV& atv) {
  return workers->enqueue([&] {
    PopCheckAtv atvCheck(alt_, atv);
    return atvCheck();
  });
}

} // namespace altintegration
