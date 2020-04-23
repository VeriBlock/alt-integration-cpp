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
    return tree_->acceptBlock(block_, state);
  }

  void UnExecute() override { tree_->invalidateBlockByHash(block_->getHash()); }

  std::string describe() const override;

 private:
  Tree* tree_;
  std::shared_ptr<Block> block_;
};

using AddBtcBlock = AddBlock<BtcBlock, BtcChainParams>;
using AddVbkBlock = AddBlock<VbkBlock, VbkChainParams>;

template <>
inline std::string AddBtcBlock::describe() const {
  return "AddBtcBlock{prevBest=" +
         tree_->getBestChain().tip()->toPrettyString() +
         ", block=" + block_->getHash().toHex() + "}";
}

template <>
inline std::string AddVbkBlock::describe() const {
  return "AddVbkBlock{prevBest=" +
         tree_->getBestChain().tip()->toPrettyString() +
         ", block=" + block_->getHash().toHex() + "}";
}

template <typename BlockTree>
bool addBlock(BlockTree& tree,
              const typename BlockTree::block_t& block,
              ValidationState& state,
              CommandHistory& history) {
  using block_t = typename BlockTree::block_t;
  using params_t = typename BlockTree::params_t;
  if (tree.getBlockIndex(block.getHash()) == nullptr) {
    // we don't know this block, create command
    auto blockPtr = std::make_shared<block_t>(block);
    auto cmd = std::make_shared<AddBlock<block_t, params_t>>(tree, blockPtr);
    return history.exec(cmd, state);
  }

  // we already added this block
  return true;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ADDBLOCK_HPP
