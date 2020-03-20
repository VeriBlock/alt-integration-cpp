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
template <typename Block, typename Payloads>
struct PayloadsWriteBatch {
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! block hash type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;

  virtual ~PayloadsWriteBatch() = default;

  /**
   * Write a single payload. If payload with such referenced block hash exists,
   *db will add new payload to the stored collection of payloads
   * @param block to be written in a batch
   */
  virtual void put(const hash_t& hash, const stored_payloads_t& payloads) = 0;

  /**
   * Remove a single block from storage identified by its hash.
   * @param hash block hash
   */
  virtual void removeByHash(const hash_t& hash) = 0;

  /**
   * Clear batch from any modifying operations.
   */
  virtual void clear() = 0;

  /**
   * Efficiently commit given batch on-disk. Clears batch from changes.
   */
  virtual void commit() = 0;
};

template <typename Block, typename Payloads>
struct PayloadsRepository {
  //! block type
  using block_t = Block;
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! block hash type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;
  //! stored payloads container type
  using stored_payloads_container_t = std::vector<stored_payloads_t>;
  //! iterator type
  using cursor_t = Cursor<hash_t, stored_payloads_container_t>;

  virtual ~PayloadsRepository() = default;

  /**
   * Load payloads from disk in memory, by block hash that cointains it.
   * @param hash[in] block hash
   * @return payloads data that is referenced to the current block hash.
   */
  virtual stored_payloads_container_t get(const hash_t& hash) const = 0;

  /**
   * Write a single payload. If payloads with referenced block hash exists,
   * db will add new payload to the stored collection of payloads.
   * @param block hash
   * @param payloads to be written in a batch
   */
  virtual void put(const hash_t& hash, const stored_payloads_t& payloads) = 0;

  /**
   * Remove a single block from storage identified by its hash.
   * @param hash block hash
   * @return true if removed, false if no such element found.
   */
  virtual void removeByHash(const hash_t& hash) = 0;

  /**
   * Clear the entire payloads data.
   */
  virtual void clear() = 0;

  /**
   * Create new WriteBatch, to perform BULK modify operations.
   * @return a pointer to new WriteBatch instance.
   */
  virtual std::unique_ptr<PayloadsWriteBatch<block_t, stored_payloads_t>>
  newBatch() = 0;

  /**
   * Returns iterator, that is used for iteration over storage.
   * @return
   */
  virtual std::shared_ptr<cursor_t> newCursor() = 0;
};

}  // namespace altintegration

#endif
