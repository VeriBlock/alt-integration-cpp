// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ORPHAN_BLOCK_CONTAINER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ORPHAN_BLOCK_CONTAINER_HPP_

#include <veriblock/assert.hpp>
#include <veriblock/signals.hpp>
#include <veriblock/strutil.hpp>
#include <veriblock/validation_state.hpp>

namespace altintegration {

/**
 * OrphanBlockContainer is a container that holds orphan blocks.
 *
 * An orphan block is a block that temporarily cannot be added to the underlying
 * block tree due to the requirement that added blocks must form a tree.
 *
 * The container immediately hands over non-orphan blocks it receives to the
 * underlying tree.
 *
 * Once the container receives blocks that connect orphan blocks to the
 * tree(making them non-orphan), it hands over these blocks to the tree.
 *
 * If the block is added successfully to the underlying tree, onBlockHandedOver
 * signal is emitted. If the block is rejected by the tree, onInvalidBlock is
 * emitted.
 *
 * @tparam Tree underlying block tree
 */
template <typename Tree>
struct OrphanBlockContainer {
  using tree_t = Tree;
  using block_t = typename Tree::block_t;
  using block_id_t = typename block_t::hash_t;
  using payloads_t = typename Tree::payloads_t;

  struct block_with_data_t {
    template <typename B, typename P>
    block_with_data_t(B&& b, P&& p)
        : block(std::forward<B>(b)), payloads(std::forward<P>(p)) {
      VBK_ASSERT(p != nullptr);
    };

    block_t block;
    std::shared_ptr<payloads_t> payloads;
  };

  using orphan_store_t =
      std::unordered_map<block_id_t, std::shared_ptr<block_with_data_t>>;
  using prev_block_index_t =
      std::unordered_multimap<block_id_t, std::shared_ptr<block_with_data_t>>;

 private:
  tree_t& _tree;
  unsigned int _blockCountWarningThreshold;
  orphan_store_t _orphanStore;
  prev_block_index_t _prevBlockIndex;

 public:
  //! a block has been handed over to the underlying tree and flagged as invalid
  signals::Signal<void(std::shared_ptr<block_with_data_t>, ValidationState&)>
      onInvalidBlock;

  //! a block has been successfully handed over to the underlying tree
  signals::Signal<void(std::shared_ptr<block_with_data_t>)> onBlockHandedOver;

  OrphanBlockContainer(tree_t& tree,
                       unsigned int blockCountWarningThreshold = 1024)
      : _tree(tree), _blockCountWarningThreshold(blockCountWarningThreshold){};

  /**
   * Pass the block header to the underlying tree
   * @return true if the block header is valid and has been added by the tree
   */
  bool acceptHeader(const block_t& block, ValidationState& state) {
    return _tree.acceptBlockHeader(block, state);
  }

  template <typename B, typename P>
  bool acceptBlock(B&& block, P&& payloads, ValidationState& state) {
    return acceptBlock(std::make_shared<block_with_data_t>(
                           std::forward<B>(block), std::forward<P>(payloads)),
                       state);
  }

  /**
   * Add the given block to the container
   * @return true if the block has been added to the container, false if the
   * block has an invalid header
   */
  bool acceptBlock(std::shared_ptr<block_with_data_t> blockData,
                   ValidationState& state) {
    VBK_LOG_DEBUG("Adding block=%s", blockData->block.toPrettyString());

    auto& block = blockData->block;

    if (!acceptHeader(block, state)) {
      return false;
    }

    if (isOrphan(block)) {
      if (size() > _blockCountWarningThreshold) {
        VBK_LOG_WARN("too many orphan blocks(%d)", size());
      }
      auto block_it = _orphanStore.find(block.getHash());
      VBK_ASSERT_MSG(block_it == _orphanStore.end(),
                     "block=%s is already in the container",
                     block.toPrettyString());

      _orphanStore.emplace(block.getHash(), blockData);
      _prevBlockIndex.emplace(block.previousBlock, std::move(blockData));
    } else {
      handOver(std::move(blockData));
    }

    return true;
  }

  /**
   * Check if a given block is an orphan (cannot be added to the underlying tree
   * temporarily)
   * @return true if the block is an orphan
   */
  bool isOrphan(const block_t& block) const {
    return !_tree.isBlockInitialized(block.previousBlock);
  }

  /**
   * Obtain read-only access to the stored orphan blocks
   * @return const reference to the orphan block container
   */
  const orphan_store_t& getOrphanBlocks() const { return _orphanStore; }

  /**
   * @return the number of orphan blocks stored in the container
   */
  size_t size() const {
    VBK_ASSERT_MSG(
        _orphanStore.size() == _prevBlockIndex.size(),
        "state corruption: the orphan block store and its index are out "
        "of sync(%d vs %d)",
        _orphanStore.size(),
        _prevBlockIndex.size());

    return _orphanStore.size();
  }

  /**
   * Check the container for consistency. Throw an assert if not consistent.
   */
  void assertConsistent() {
    // trigger the size mismatch assert
    std::ignore = size();

    // check that each _orphanStore entry is consistent and has the
    // corresponding entry in _prevBlockIndex
    for (const auto& orphan : _orphanStore) {
      VBK_ASSERT_MSG(orphan.first == orphan.second->block.getHash(),
                     "the hash of the orphan block entry(%s) does not match "
                     "the hash of the block=%s",
                     HexStr(orphan.first),
                     orphan.second->block.toPrettyString());

      auto prevBlockRange =
          _prevBlockIndex.equal_range(orphan.second->block.previousBlock);

      auto entry =
          std::find_if(prevBlockRange.first,
                       prevBlockRange.second,
                       [&](const typename prev_block_index_t::reference item) {
                         return item.second == orphan.second;
                       });

      VBK_ASSERT_MSG(entry != prevBlockRange.second,
                     "orphan store entry=%s does not have the corresponding "
                     "previous block index entry(%s)",
                     orphan.second->block.toPrettyString());
    }

    // check that each entry in _prevBlockIndex is consistent and has the
    // corresponding _orphanStore entry
    for (const auto& prevBlock : _prevBlockIndex) {
      VBK_ASSERT_MSG(prevBlock.first == prevBlock.second->block.previousBlock,
                     "the hash of the previous block index entry(%s) does not "
                     "match the block header=%s",
                     HexStr(prevBlock.first),
                     prevBlock.second->block.toPrettyString());

      auto orphan = _orphanStore.find(prevBlock.second->block.getHash());
      VBK_ASSERT_MSG(orphan != _orphanStore.end(),
                     "previous block index entry(%s) does not have the "
                     "corresponding orphan store entry(%s)",
                     HexStr(prevBlock.first),
                     prevBlock.second->block.toPrettyString());
    }
  }

 private:
  void handOver(std::shared_ptr<block_with_data_t> blockData) {
    VBK_LOG_DEBUG("Handing over block=%s to the underlying tree",
                  blockData->block.toPrettyString());

    auto blockHash = blockData->block.getHash();

    ValidationState state;
    bool added = _tree.addPayloads(blockHash, *blockData->payloads, state);

    if (added) {
      VBK_LOG_DEBUG("Successfully handed over block=%s",
                    blockData->block.toPrettyString());
      onBlockHandedOver.emit(std::move(blockData));
    } else {
      VBK_LOG_DEBUG("Failed to hand over block=%s: %s",
                    blockData->block.toPrettyString(),
                    state.toString());
      onInvalidBlock.emit(std::move(blockData), state);
    }

    // hand over the successors and remove them from the store
    auto successors = _prevBlockIndex.equal_range(blockHash);
    // UGLY: have to compare with both end() and the end of the range as they
    // are not equal and both can terminate the range
    for (auto successor = successors.first;
         successor != successors.second && successor != _prevBlockIndex.end();
         successor = _prevBlockIndex.erase(successor)) {
      handOver(successor->second);

      auto erasedCount = _orphanStore.erase(successor->second->block.getHash());
      VBK_ASSERT_MSG(erasedCount == 1,
                     "attempted to hand over a block(%s) we don't have in the "
                     "orphan store",
                     successor->second->block.toPrettyString());
    }
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ORPHAN_BLOCK_CONTAINER_HPP_
