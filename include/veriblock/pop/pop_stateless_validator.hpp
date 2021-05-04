// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_POP_STATELESS_VALIDATOR_HPP
#define ALT_INTEGRATION_VERIBLOCK_POP_STATELESS_VALIDATOR_HPP

#include <future>
#include <veriblock/pop/blockchain/alt_chain_params.hpp>
#include <veriblock/pop/blockchain/btc_chain_params.hpp>
#include <veriblock/pop/blockchain/vbk_chain_params.hpp>
#include <veriblock/pop/validation_state.hpp>

#ifndef VBK_NO_THREADS
#include <veriblock/pop/third_party/thread_pool.hpp>
#endif

namespace altintegration {

#ifndef VBK_NO_THREADS
//! @private
using ValidationThreadPool =
    tp::ThreadPoolImpl<tp::FixedFunction<void(), 128>, tp::MPMCBoundedQueue>;
#endif

//! @private
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

  const VbkChainParams& getVbkParams() const { return vbk_; }
  const BtcChainParams& getBtcParams() const { return btc_; }
  const AltChainParams& getAltParams() const { return alt_; }

  void clear();

 protected:
  size_t threads_ = 0;
#ifndef VBK_NO_THREADS
  std::shared_ptr<ValidationThreadPool> workers;
#endif
  const VbkChainParams& vbk_;
  const BtcChainParams& btc_;
  const AltChainParams& alt_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_VERIBLOCK_POP_STATELESS_VALIDATOR_HPP
