// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_

#include <veriblock/storage/payloads_repository_inmem.hpp>

namespace altintegration {

template <typename Payloads>
class EndorsementStorage {
  using payloads_t = Payloads;
  using eid_t = typename Payloads::id_t;

 public:
  virtual ~EndorsementStorage() = default;

  EndorsementStorage()
      : prepo_(
            std::move(std::make_shared<PayloadsRepositoryInmem<Payloads>>())) {}

  PayloadsRepository<Payloads>& payloads() { return *prepo_; }
  const PayloadsRepository<Payloads>& payloads() const { return *prepo_; }

 private:
  std::shared_ptr<PayloadsRepository<Payloads>> prepo_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_