// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_BTC_ENDORSEMENT_REPOSITORY_MOCK_H
#define ALT_INTEGRATION_BTC_ENDORSEMENT_REPOSITORY_MOCK_H

#include <veriblock/storage/endorsement_repository.hpp>

namespace altintegration {

template <typename Endorsement>
struct EndorsementRepositoryMock : public EndorsementRepository<Endorsement> {
  using endorsed_hash_t = typename Endorsement::endorsed_hash_t;
  using containing_hash_t = typename Endorsement::containing_hash_t;
  using container_t = typename Endorsement::container_t;
  using eid_t = typename Endorsement::id_t;

  ~EndorsementRepositoryMock() override = default;

  MOCK_METHOD1_T(remove, void(const eid_t&));
  MOCK_METHOD1_T(remove, void(const container_t&));

  MOCK_METHOD1_T(put, void(const container_t&));

  MOCK_CONST_METHOD1_T(get, std::vector<Endorsement>(const endorsed_hash_t&));
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_BTC_ENDORSEMENT_REPOSITORY_MOCK_H
