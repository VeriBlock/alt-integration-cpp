#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_HPP_

#include <cstdint>
#include <memory>
#include <vector>
#include <veriblock/slice.hpp>
#include <veriblock/storage/cursor.hpp>

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

// forward decl
template <typename Block>
struct BlockRepository;

/**
 * @class WriteBatch
 * @brief Efficiently implements bulk write operation for BlockRepository.
 *
 * @invariant WriteBatch is always in valid state.
 * @invariant WriteBatch does not modify on-disk storage after put/remove
 * operations. It does, when \p BlockRepository::commit is executed on this
 * batch.
 */
template <typename Block>
struct WriteBatch {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! iterator type
  using cursor_t = Cursor<hash_t, stored_block_t>;

  virtual ~WriteBatch() = default;

  /**
   * Write a single block. If block with such hash exists, db will overwrite
   * it.
   * @param block to be written in a batch
   * @return true if we overwrite existing block, false otherwise.
   */
  virtual void put(const stored_block_t& block) = 0;

  /**
   * Remove a single block from storage identified by its hash.
   * @param hash block hash
   * @return true if removed, false if no such element found.
   */
  virtual void removeByHash(const hash_t& hash) = 0;

  /**
   * Clear batch from any modifying operations.
   */
  virtual void clear() = 0;

  /**
   * Efficiently commit given batch on-disk. Clears batch from changes.
   * @param repo
   */
  virtual void commit(BlockRepository<Block>& repo) = 0;
};

/**
 * @class BlockRepository
 *
 * @brief Represents a block tree stored on disk.
 *
 * Multiple blocks may be stored at the given height.
 *
 * @tparam Block Stored Block type
 *
 * @invariant any instance is in valid state - database is opened when valid
 * instance of BlockRepository created. To close database, delete the instance.
 */
template <typename Block>
struct BlockRepository {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;
  //! iterator type
  using cursor_t = Cursor<hash_t, stored_block_t>;

  virtual ~BlockRepository() = default;

  /**
   * Load a block from disk in memory, by its hash.
   * @param hash[in] block hash
   * @param out[out] if non-null, block data will be written here. If null
   * passed, out argument is ignored.
   * @return true if block found, false otherwise.
   */
  virtual bool getByHash(const hash_t& hash, stored_block_t* out) const = 0;

  /**
   * Load many blocks from disk in memory by a list of hashes.
   * @param hashes[in] a list of hashes to load.
   * @param out[out] if non-null, blocks will be appended to this vector. If
   * null passed, out argument is ignored.
   * @return number of blocks appended to output vector.
   */
  virtual size_t getManyByHash(Slice<const hash_t> hashes,
                               std::vector<stored_block_t>* out) const = 0;

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
   * Create new WriteBatch, to perform BULK modify operations.
   * @return a pointer to new WriteBatch instance.
   */
  virtual std::unique_ptr<WriteBatch<stored_block_t>> newBatch() = 0;

  /**
   * Returns iterator, that is used for height iteration over blockchain.
   * @return
   */
  virtual std::shared_ptr<cursor_t> newCursor() = 0;

  bool contains(const hash_t& hash) const { return getByHash(hash, nullptr); }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_HPP_
