#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_IBLOCKCHAIN_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_IBLOCKCHAIN_HPP_

#include <memory>
#include <unordered_set>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/btc_fork_resolution.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/stateless_validation.hpp>
#include <veriblock/storage/block_repository.hpp>
#include <veriblock/validation_state.hpp>

namespace VeriBlock {

/**
 * Blockchain is a tree of blocks with single "bootstrap" block as root.
 * @tparam Block
 */
template <typename Block>
struct IBlockchain {
  using block_t = Block;
  using index_t = BlockIndex<block_t>;
  using hash_t = typename Block::hash_t;
  using height_t = typename Block::height_t;

  virtual ~IBlockchain() = default;

  /**
   * Bootstrap blockchain with a single bootstrap block on given height.
   *
   * This function does all blockchain integrity checks, does blockchain cleanup
   * and in general, very slow.
   *
   * @param height bootstrap block height
   * @param block bootstrap block header
   * @return true if blockchain has been loaded successfully. False, if
   * blockchain stored in BlockRepository had some invalid data.
   */
  virtual bool bootstrap(height_t height, const block_t& block, ValidationState& state) = 0;
  virtual bool acceptBlock(const block_t& block, ValidationState& state) = 0;
  virtual const Chain<Block>& getBestChain() const = 0;
  virtual index_t* getBlockIndex(const hash_t& hash) = 0;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_IBLOCKCHAIN_HPP_
