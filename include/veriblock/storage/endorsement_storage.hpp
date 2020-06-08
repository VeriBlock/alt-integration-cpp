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
  using endorsement_t = typename decltype(Payloads::containingBlock)::endorsement_t;
  using pid_t = typename Payloads::id_t;

 public:
  virtual ~EndorsementStorage() = default;

  EndorsementStorage()
      : prepo_(
            std::move(std::make_shared<PayloadsRepositoryInmem<Payloads>>())),
        erepo_(
            std::move(std::make_shared<PayloadsRepositoryInmem<endorsement_t>>())) {}

  PayloadsRepository<Payloads>& payloads() { return *prepo_; }
  const PayloadsRepository<Payloads>& payloads() const { return *prepo_; }

  PayloadsRepository<endorsement_t>& endorsements() { return *erepo_; }
  const PayloadsRepository<endorsement_t>& endorsements() const {
    return *erepo_;
  }

  bool getPayloadsById(const pid_t& id, Payloads* payloads) {
    return prepo_->get(id, payloads);
  }

 private:
  std::shared_ptr<PayloadsRepository<Payloads>> prepo_;
  std::shared_ptr<PayloadsRepository<endorsement_t>> erepo_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_