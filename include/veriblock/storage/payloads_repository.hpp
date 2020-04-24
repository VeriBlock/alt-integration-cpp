// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_HPP_

#include <memory>
#include <vector>

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
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! payloads id type
  using payloads_id = typename Payloads::id_t;

  virtual ~PayloadsWriteBatch() = default;

  /**
   * Write a single payload. If payload with such referenced block hash exists,
   *db will add new payload to the stored collection of payloads
   * @param block to be written in a batch
   */
  virtual void put(const stored_payloads_t& payloads) = 0;

  /**
   * Remove a single block from storage identified by its hash.
   * @param hash block hash
   */
  virtual void removeByHash(const payloads_id& hash) = 0;

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
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! payloads id type
  using payloads_id = typename Payloads::id_t;
  //! iterator type
  using cursor_t = Cursor<payloads_id, stored_payloads_t>;

  virtual ~PayloadsRepository() = default;

  virtual bool get(const payloads_id& id, stored_payloads_t* out) const = 0;

  virtual size_t get(const std::vector<payloads_id>& ids,
                     std::vector<stored_payloads_t>* out) const = 0;

  virtual void put(const stored_payloads_t& payloads) = 0;

  virtual void removeByHash(const payloads_id& id) = 0;

  /**
   * Clear the entire payloads data.
   */
  virtual void clear() = 0;

  /**
   * Create new WriteBatch, to perform BULK modify operations.
   * @return a pointer to new WriteBatch instance.
   */
  virtual std::unique_ptr<PayloadsWriteBatch<stored_payloads_t>> newBatch() = 0;

  /**
   * Returns iterator, that is used for iteration over storage.
   * @return
   */
  virtual std::shared_ptr<cursor_t> newCursor() = 0;
};

}  // namespace altintegration

#endif
