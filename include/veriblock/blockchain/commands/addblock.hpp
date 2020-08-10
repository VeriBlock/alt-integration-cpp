// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ADDBLOCK_HPP
#define ALTINTEGRATION_ADDBLOCK_HPP

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/command.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/vbkblock.hpp>

#include "veriblock/fmt.hpp"

namespace altintegration {

template <typename Block, typename ChainParams>
struct AddBlock : public Command {
  using Tree = BlockTree<Block, ChainParams>;
  using ref_height_t = int32_t;

  AddBlock(Tree& tree,
           std::shared_ptr<Block> block,
           ref_height_t referencedAtHeight = 0)
      : tree_(&tree),
        block_(std::move(block)),
        referencedAtHeight_(referencedAtHeight) {}

  bool Execute(ValidationState& state) override {
    auto* index = tree_->getBlockIndex(block_->getHash());

    if (index == nullptr) {
      if (!tree_->acceptBlock(block_, state)) {
        return false;
      }

      index = tree_->getBlockIndex(block_->getHash());
      VBK_ASSERT(index != nullptr &&
                 "could not find the block we have just added");
    }

    index->addRef(referencedAtHeight_);
    return true;
  }

  void UnExecute() override {
    auto hash = block_->getHash();
    auto* index = tree_->getBlockIndex(hash);
    VBK_ASSERT_MSG(index != nullptr,
                   "failed to roll back AddBlock: the block does not exist %s",
                   HexStr(hash));

    index->removeRef(referencedAtHeight_);

    if (index->refCount() == 0) {
      assertBlockCanBeRemoved(*index);
      return tree_->removeLeaf(*index);
    }
  }

  size_t getId() const override { return block_->getHash().getLow64(); }

  std::string toPrettyString(size_t level = 0) const override;

 private:
  Tree* tree_;
  std::shared_ptr<Block> block_;
  int32_t referencedAtHeight_;
};

using AddBtcBlock = AddBlock<BtcBlock, BtcChainParams>;
using AddVbkBlock = AddBlock<VbkBlock, VbkChainParams>;

template <>
inline std::string AddBtcBlock::toPrettyString(size_t level) const {
  return fmt::sprintf("%sAddBtcBlock{prev=%s, block=%s}",
                      std::string(level, ' '),
                      block_->previousBlock.toHex(),
                      block_->getHash().toHex());
}

template <>
inline std::string AddVbkBlock::toPrettyString(size_t level) const {
  return fmt::sprintf("%sAddVbkBlock{prev=%s, block=%s, height=%ld}",
                      std::string(level, ' '),
                      block_->previousBlock.toHex(),
                      block_->getHash().toHex(),
                      block_->height);
}

template <typename BlockTree>
void addBlock(BlockTree& tree,
              const typename BlockTree::block_t& block,
              int32_t referencedAtHeight,
              std::vector<CommandPtr>& commands) {
  using block_t = typename BlockTree::block_t;
  using params_t = typename BlockTree::params_t;
  // we don't know this block, create command
  auto blockPtr = std::make_shared<block_t>(block);
  auto cmd = std::make_shared<AddBlock<block_t, params_t>>(
      tree, std::move(blockPtr), referencedAtHeight);
  commands.push_back(std::move(cmd));
};

template <typename BlockTree>
void addBlock(BlockTree& tree,
              const typename BlockTree::block_t& block,
              std::vector<CommandPtr>& commands) {
  return addBlock(tree, block, 0, commands);
}

}  // namespace altintegration

#endif  // ALTINTEGRATION_ADDBLOCK_HPP
