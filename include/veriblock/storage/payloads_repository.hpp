// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_HPP_

#include <functional>
#include <string>
#include <vector>

#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/endorsement.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbkpoptx.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/storage/cursor.hpp"

namespace altintegration {

/**
 * @class WriteBatch
 * @brief Efficiently implements bulk write operation for PayloadsRepository.
 *
 * @invariant PayloadsWriteBatch is always in valid state.
 * @invariant PayloadsWriteBatch does not modify on-disk storage after
 * put/remove operations.
 */
template <typename Payloads>
struct PayloadsWriteBatch {
  using payloads_t = Payloads;
  using eid_t = typename Payloads::id_t;

  virtual ~PayloadsWriteBatch() = default;

  virtual void put(const payloads_t& payload) = 0;

  virtual void remove(const eid_t& id) = 0;

  /**
   * Clear batch from any modifying operations.
   */
  virtual void clear() = 0;

  /**
   * Efficiently commit given batch on-disk. Clears batch from changes.
   */
  virtual void commit() = 0;
};

template <typename Payloads>
struct PayloadsRepository {
  using payloads_t = Payloads;
  using eid_t = typename Payloads::id_t;
  using cursor_t = Cursor<eid_t, Payloads>;

  virtual ~PayloadsRepository() = default;

  virtual bool remove(const eid_t& id) = 0;

  virtual bool put(const payloads_t& payload) = 0;

  virtual bool get(const eid_t& id, payloads_t* payload = 0) const = 0;

  virtual std::unique_ptr<PayloadsWriteBatch<Payloads>> newBatch() = 0;

  virtual std::shared_ptr<cursor_t> newCursor() const = 0;
};

template <typename Payloads>
void payloadsRepositoryCopy(const PayloadsRepository<Payloads>& copy_from,
                            PayloadsRepository<Payloads>& copy_to) {
  auto cursor = copy_from.newCursor();

  for (cursor->seekToFirst(); cursor->isValid(); cursor->next()) {
    copy_to.put(cursor->value());
  }
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_HPP_
