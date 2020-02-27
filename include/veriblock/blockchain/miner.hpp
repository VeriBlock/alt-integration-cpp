#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_HPP_

#include <memory>
#include <stdexcept>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/fmt.hpp>
#include <veriblock/stateless_validation.hpp>

namespace VeriBlock {

template <typename Block, typename ChainParams>
struct Miner {
  using merkle_t = decltype(Block::merkleRoot);
  using hash_t = decltype(Block::previousBlock);
  using index_t = BlockIndex<Block>;

  Miner(std::shared_ptr<ChainParams> params, uint32_t startTime)
      : params_(std::move(params)), startTime_(startTime) {}

  void createBlock(Block& block) {
    while (!checkProofOfWork(block, *params_)) {
      ++block.nonce;
      if (block.nonce >= std::numeric_limits<decltype(block.nonce)>::max()) {
        ++block.timestamp;
        block.nonce = 0;
      }
    }
  }

  // One must define their own template specialization for given Block and
  // ChainParams types. Otherwise, get pretty compilation error.
  Block getBlockTemplate(const BlockIndex<Block>& tip,
                         const merkle_t& merkleRoot);

  Block createNextBlock(const index_t& prev, const merkle_t& merkle) {
    Block block = getBlockTemplate(prev, merkle);
    createBlock(block);
    return block;
  }

 private:
  std::shared_ptr<ChainParams> params_;
  uint32_t startTime_;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_HPP_
