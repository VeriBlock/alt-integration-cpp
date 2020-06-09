// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_

#include <veriblock/storage/payloads_repository_inmem.hpp>

namespace altintegration {

template <typename Endorsements>
class EndorsementStorage {
  using endorsement_t = Endorsements;

 public:
  virtual ~EndorsementStorage() = default;

  EndorsementStorage()
      : erepo_(
            std::move(std::make_shared<PayloadsRepositoryInmem<endorsement_t>>())) {}

  PayloadsRepository<endorsement_t>& endorsements() { return *erepo_; }
  const PayloadsRepository<endorsement_t>& endorsements() const {
    return *erepo_;
  }

 protected:
  std::shared_ptr<PayloadsRepository<endorsement_t>> erepo_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_