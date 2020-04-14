#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_HPP_

#include <memory>
#include <random>
#include <stdexcept>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/stateless_validation.hpp>

namespace altintegration {

template <typename Block, typename ChainParams>
struct Miner {
  using merkle_t = decltype(Block::merkleRoot);
  using index_t = BlockIndex<Block>;

  Miner(const ChainParams& params) : params_(params) {}

  void createBlock(Block& block) {
    while (!checkProofOfWork(block, params_)) {
      // to guarantee that miner will not create exactly same blocks even if
      // time and merkle roots are equal for prev block and new block
      block.nonce = nonce++;
      if (block.nonce >= std::numeric_limits<decltype(block.nonce)>::max()) {
        ++block.timestamp;
        nonce = 0;
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

  Block createNextBlock(const index_t& prev) {
    merkle_t merkle;
    std::generate(
        merkle.begin(), merkle.end(), []() { return uint8_t(rand() & 0xff); });
    return createNextBlock(prev, merkle);
  }

 private:
  const ChainParams& params_;
  uint32_t nonce = 0;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_HPP_
