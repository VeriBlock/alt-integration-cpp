// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ADDBLOCK_HPP
#define ALTINTEGRATION_ADDBLOCK_HPP

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/command.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/vbkblock.hpp>

namespace altintegration {

template <typename Block, typename ChainParams>
struct AddBlock : public Command {
  using Tree = BlockTree<Block, ChainParams>;
  AddBlock(Tree& tree, std::shared_ptr<Block> block)
      : tree_(&tree), block_(std::move(block)) {}

  bool Execute(ValidationState& state) override {
    auto* index = tree_->getBlockIndex(block_->getHash());
    if (index) {
      index->refCounter++;
      return true;
    }
    return tree_->acceptBlock(block_, state);
  }

  void UnExecute() override {
    auto hash = block_->getHash();
    auto* index = tree_->getBlockIndex(hash);
    if (!index) {
      return;
    }

    if (index->refCounter == 0) {
      return tree_->removeSubtree(block_->getHash());
    }

    --index->refCounter;
  }

  size_t getId() const override { return block_->getHash().getLow64(); }

  std::string toPrettyString(size_t level = 0) const override;

 private:
  Tree* tree_;
  std::shared_ptr<Block> block_;
};

using AddBtcBlock = AddBlock<BtcBlock, BtcChainParams>;
using AddVbkBlock = AddBlock<VbkBlock, VbkChainParams>;

template <>
inline std::string AddBtcBlock::toPrettyString(size_t level) const {
  return std::string(level, ' ') +
         "AddBtcBlock{prev=" + block_->previousBlock.toHex() +
         ", block=" + block_->getHash().toHex() + "}";
}

template <>
inline std::string AddVbkBlock::toPrettyString(size_t level) const {
  return std::string(level, ' ') +
         "AddVbkBlock{prev=" + block_->previousBlock.toHex() +
         ", block=" + block_->getHash().toHex() +
         ", height=" + std::to_string(block_->height) + "}";
}

template <typename BlockTree>
void addBlock(BlockTree& tree,
              const typename BlockTree::block_t& block,
              std::vector<CommandPtr>& commands) {
  using block_t = typename BlockTree::block_t;
  using params_t = typename BlockTree::params_t;
  // we don't know this block, create command
  auto blockPtr = std::make_shared<block_t>(block);
  auto cmd =
      std::make_shared<AddBlock<block_t, params_t>>(tree, std::move(blockPtr));
  commands.push_back(std::move(cmd));
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ADDBLOCK_HPP