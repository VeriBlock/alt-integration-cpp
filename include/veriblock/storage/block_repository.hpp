#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_HPP_

#include <cstdint>
#include <memory>
#include <vector>
#include <veriblock/slice.hpp>

namespace VeriBlock {

namespace db {
/**
 * @class UnrecoverableError
 * @brief Application is not able to recover from the error occurred in the
 * storage.
 *
 * Example: database is corrupted and can not be opened.
 *
 * It is up to implementation to decide a full list of such error types.
 */
struct UnrecoverableError : public std::exception {};

/**
 * @class Error
 * @brief Application in normal condition can recover from this type of error.
 *
 * Example: out of disk space, IO error.
 *
 * It is up to implementation to decide a full list of such error types.
 */
struct Error : public std::exception {};
}  // namespace db

/**
 * @class BlockRepository
 *
 * @brief Represents a block tree stored on disk.
 *
 * Multiple blocks may be stored at the given height.
 *
 * @tparam Block Stored Block type
 *
 * @invariant any instance is invalid state - database is opened when valid
 * instance of BlockRepository created. To close database, delete the instance.
 */
template <typename Block>
struct BlockRepository {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! block heiehgt type
  using height_t = typename Block::height_t;

  virtual ~BlockRepository() = default;

  /**
   * @class WriteBatch
   * @brief Efficiently implements bulk write operation for BlockRepository.
   *
   * @invariant WriteBatch is always in valid state.
   * @invariant WriteBatch does not modify on-disk storage after put/remove
   * operations. It does, when \p BlockRepository::commit is executed on this
   * batch.
   */
  struct WriteBatch {
    virtual ~WriteBatch() = default;

    /**
     * Write a single block. If block with such hash exists, db will overwrite
     * it.
     * @param block to be written in a batch
     * @return true if block already existed in db and we overwrote it. False
     * otherwise.
     */
    virtual bool put(const stored_block_t& block) = 0;

    /**
     * Remove a single block from storage identified by its hash.
     * @param hash block hash
     * @return true if removed, false if no such element found.
     */
    virtual bool removeByHash(const hash_t& hash) = 0;

    /**
     * Remove potentially many blocks at given height.
     * @param height block height
     * @return true if at least one block removed, false otherwise.
     */
    virtual bool removeByHeight(height_t height) = 0;

    /**
     * Clear batch from any modifying operations.
     */
    virtual void clear() = 0;
  };

  /**
   * Load a block from disk in memory, by its hash.
   * @param hash[in] block hash
   * @param out[out] if non-null, block data will be written here. If null
   * passed, out argument is ignored.
   * @return true if block found, false otherwise.
   */
  virtual bool getByHash(const hash_t& hash, stored_block_t* out) = 0;

  /**
   * Load potentially many blocks from disk in memory by their height.
   * @param height[in] block height
   * @param out[out] if non-null, blocks will be written here. If null passed,
   * out argument is ignored.
   * @return true if block found at given height, false otherwise.
   */
  virtual bool getByHeight(const height_t& height,
                           std::vector<stored_block_t>* out) = 0;

  /**
   * Load many blocks from disk in memory by a list of hashes.
   * @param hashes[in] a list of hashes to load.
   * @param out[out] if non-null, blocks will be written here. If null passed,
   * out argument is ignored.
   * @return true if at least one block has been found, false otherwise.
   */
  virtual bool getManyByHash(Slice<const hash_t> hashes,
                             std::vector<stored_block_t>* out) = 0;

  /**
   * Write a single block. If block with such hash exists, db will overwrite
   * it.
   * @param block to be written in a batch
   * @return true if block already existed in db and we overwrote it. False
   * otherwise.
   */
  virtual void put(const stored_block_t& block) = 0;

  /**
   * Remove a single block from storage identified by its hash.
   * @param hash block hash
   * @return true if removed, false if no such element found.
   */
  virtual bool removeByHash(const hash_t& hash) = 0;

  /**
   * Remove potentially many blocks at given height.
   * @param height block height
   * @return true if at least one block removed, false otherwise.
   */
  virtual int removeByHeight(height_t height) = 0;

  /**
   * Create new WriteBatch, to perform BULK modify operations.
   * @return a pointer to new WriteBatch instance.
   */
  virtual std::unique_ptr<WriteBatch> getBatch() = 0;

  /**
   * Efficiently commit given batch on-disk.
   * @param batch
   */
  virtual void commit(WriteBatch& batch) = 0;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_HPP_
