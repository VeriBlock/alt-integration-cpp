// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_POP_STATELESS_VALIDATOR_HPP
#define ALT_INTEGRATION_VERIBLOCK_POP_STATELESS_VALIDATOR_HPP

#include <veriblock/config.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/third_party/ThreadPool.h>

namespace altintegration {

class PopCheckBlock {
 public:
  PopCheckBlock(const VbkChainParams& vbk, const VbkBlock& block)
      : vbk_(vbk), block_(block) {}

  ValidationState operator()();

 protected:
  const VbkChainParams& vbk_;
  const VbkBlock& block_;
};

class PopCheckVtb {
 public:
  PopCheckVtb(const BtcChainParams& btc, const VTB& vtb)
      : btc_(btc), vtb_(vtb) {}

  ValidationState operator()();

 protected:
  const BtcChainParams& btc_;
  const VTB& vtb_;
};

class PopCheckAtv {
 public:
  PopCheckAtv(const AltChainParams& alt, const ATV& atv)
      : alt_(alt), atv_(atv) {}

  ValidationState operator()();

 protected:
  const AltChainParams& alt_;
  const ATV& atv_;
};

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
