#ifndef ALT_INTEGRATION_VERIBLOCK_BLOCK_STORAGE_ENDORSMENT_REPOSITORY_HPP
#define ALT_INTEGRATION_VERIBLOCK_BLOCK_STORAGE_ENDORSMENT_REPOSITORY_HPP

#include <memory>
#include <vector>

namespace VeriBlock {

template <typename Endorsement, typedef AltChainBlock>
struct BlockEndorsmentRepository {
  //! stored block type
  using stored_block_t = AltChainBlock;
  //! block hash type
  using block_hash_t = typename AltChainBlock::hash_t;
  //! stored_endorsment_t type
  using stored_endorsement_t = Endorsement;
  //! endorsment hash type
  using endorsement_hash_t = typename Endorsement::hash_t;

  /**
   * @class WriteBatch
   * @brief Efficiently implements bulk write operation for
   * BlockEndorsmentRepository.
   *
   * @invariant WriteBatch is always in valid state.
   * @invariant WriteBatch does not modify on-disk storage after put/remove
   * operations. It does, when \p BlockEndorsmentRepository::commit is executed
   * on this batch.
   */
  struct WriteBatch {
    virtual ~WriteBatch() = default;

    /**
     * Write a single endorsement which endorses a endorsed_block and contains
     * in the containig_block. If endorsement exist, db overwrites it. it.
     * @param endorsement to be written in a batch
     * @param containing_block to be written in a batch
     * @param endorsed_block to be written in a batch
     * @return true if we overwrite existing data, false otherwise.
     */
    virtual bool put(const stored_endorsement_t& endorsement,
                     const stored_block_t& containing_block,
                     const stored_block_t& endorsed_block) = 0;

    /**
     * Remove a single endorsement with all references to the containing_blocks
     * and endorsed_blocks from storage identified by endorsement hash.
     * @param hash endoresement hash
     * @return true if removed, false if no such element found.
     */
    virtual bool removeEndorsmentByHash(const endorsement_hash_t& hash) = 0;

    /**
     * Remove a single containing_block with all references to the endorsement
     * and romove all endorsements and endorsed_blocks from storage identified
     * by containing_block hash.
     * @param hash block hash
     * @return true if removed, false if no such element found.
     */
    virtual bool removeContainingBlockByHash(const block_hash_t& hash) = 0;

    /**
     * Remove a single endorsed_block with all references to the endorsement and
     * endorsed_blocks from storage identified by containing_block hash.
     * @param hash block hash
     * @return true if removed, false if no such element found.
     */
    virtual bool removeEndorsedBlockByHash(const block_hash_t& hash) = 0;

    /**
     * Clear batch from any modifying operations.
     */
    virtual void clear() = 0;
  };

  /**
   * Write a single endorsement which endorses a endorsed_block and contains
   * in the containig_block. If endorsement exist, db overwrites it. it.
   * @param endorsement to be written in a batch
   * @param containing_block to be written in a batch
   * @param endorsed_block to be written in a batch
   * @return true if we overwrite existing data, false otherwise.
   */
  virtual bool put(const stored_endorsement_t& endorsement,
                   const stored_block_t& containing_block,
                   const stored_block_t& endorsed_block) = 0;

  /**
   * Remove a single endorsement with all references to the containing_blocks
   * and endorsed_blocks from storage identified by endorsement hash.
   * @param hash endoresement hash
   * @return true if removed, false if no such element found.
   */
  virtual bool removeEndorsmentByHash(const endorsement_hash_t& hash) = 0;

  /**
   * Remove a single containing_block with all references to the endorsement
   * and romove all endorsements and endorsed_blocks from storage identified
   * by containing_block hash.
   * @param hash block hash
   * @return true if removed, false if no such element found.
   */
  virtual bool removeContainingBlockByHash(const block_hash_t& hash) = 0;

  /**
   * Remove a single endorsed_block with all references to the endorsement and
   * endorsed_blocks from storage identified by containing_block hash.
   * @param hash block hash
   * @return true if removed, false if no such element found.
   */
  virtual bool removeEndorsedBlockByHash(const block_hash_t& hash) = 0;

  /**
   * Retrieve endorsements from the given containing blocks that endorse the
   * given endorsed block
   * @param endorsed_block_hash[in] the endorsed block hash
   * @param contaning_block_hashes[in] the containing block hashes
   * @param endorsements[out] if non-null, endorsements is written here. If null
   * is passed, out arguments is ignored.
   * @return number of endorsements appended to output vector.
   */
  virtual size_t getEndorsements(
      const block_hash_t& endorsed_block_hash,
      const std::vector<block_hash_t>& contaning_block_hashes,
      std::vector<stored_endorsement_t>* endorsements) = 0;

  /**
   * Retrieve Endorsements from the given containing block
   * @param containing_block_hash[in] the containing block hash
   * @param endorsements[out] if non-null, endorsements is written here. If null
   * is passed, out arguments is ignored.
   * @return number of endorsements appended to output vector.
   */
  virtual size_t getEndorsements(
      const block_hash_t& containing_block_hash,
      std::vector<stored_endorsement_t>* endorsements) = 0;

  /**
   * Create new WriteBatch, to perform BULK modify operations.
   * @return a pointer to new WriteBatch instance.
   */
  virtual std::unique_ptr<WriteBatch> newBatch() = 0;

  /**
   * Efficiently commit given batch on-disk.
   * @param batch
   */
  virtual void commit(WriteBatch& batch) = 0;
};

}  // namespace VeriBlock
#endif
