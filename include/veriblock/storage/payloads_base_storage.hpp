// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_BASE_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_BASE_STORAGE_HPP_

#include <veriblock/storage/payloads_repository_inmem.hpp>

namespace altintegration {

template <typename Payloads>
class PayloadsBaseStorage {
  using payloads_t = Payloads;

 public:
  virtual ~PayloadsBaseStorage() = default;

  PayloadsBaseStorage()
      : prepo_(std::move(
            std::make_shared<PayloadsRepositoryInmem<payloads_t>>())) {}

 protected:
  std::shared_ptr<PayloadsRepository<payloads_t>> prepo_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_BASE_STORAGE_HPP_