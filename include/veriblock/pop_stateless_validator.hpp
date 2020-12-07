// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_POP_STATELESS_VALIDATOR_HPP
#define ALT_INTEGRATION_VERIBLOCK_POP_STATELESS_VALIDATOR_HPP

#include <future>
#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/validation_state.hpp>

namespace third_party {
class ThreadPool;
}

namespace altintegration {

class PopValidator {
 public:
  PopValidator(const VbkChainParams& vbk,
               const BtcChainParams& btc,
               const AltChainParams& alt,
               size_t threads = 0);
  ~PopValidator();

  void start(size_t threads = 0);
  void stop();

  template <typename CheckType>
  std::future<ValidationState> addCheck(const CheckType& block);

  const VbkChainParams& getVbkParams() const { return vbk_;}
  const BtcChainParams& getBtcParams() const { return btc_;}
  const AltChainParams& getAltParams() const { return alt_;}

 protected:
  std::shared_ptr<third_party::ThreadPool> workers;
  const VbkChainParams& vbk_;
  const BtcChainParams& btc_;
  const AltChainParams& alt_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_VERIBLOCK_POP_STATELESS_VALIDATOR_HPP
