// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_POP_STATELESS_VALIDATOR_HPP
#define ALT_INTEGRATION_VERIBLOCK_POP_STATELESS_VALIDATOR_HPP

#include <veriblock/third_party/ThreadPool.h>

namespace altintegration {

class PopValidator {
 public:
  PopValidator(const VbkChainParams& vbk,
               const BtcChainParams& btc,
               const AltChainParams& alt,
               size_t threads = 0);
  ~PopValidator() {}

  template <typename CheckType>
  std::future<ValidationState> addCheck(const CheckType& block);

 protected:
  std::shared_ptr<ThreadPool> workers;
  const VbkChainParams& vbk_;
  const BtcChainParams& btc_;
  const AltChainParams& alt_;
};

} // namespace altintegration

#endif //ALT_INTEGRATION_VERIBLOCK_POP_STATELESS_VALIDATOR_HPP
