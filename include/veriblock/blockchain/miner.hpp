#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_HPP_

#include <memory>
#include <stdexcept>
#include <veriblock/blockchain/blockchain.hpp>
#include <veriblock/fmt.hpp>
#include <veriblock/stateless_validation.hpp>
#include <veriblock/time.hpp>

namespace VeriBlock {

template <typename Block>
void createBlock(Block& block) {
  ValidationState state;
  while (!checkProofOfWork(block, state)) {
    ++block.nonce;
    if (block.nonce >= std::numeric_limits<decltype(block.nonce)>::max()) {
      ++block.timestamp;
    }
  }
}

template <typename Block, typename ChainParams>
struct Miner {
  using merkle_t = decltype(Block::merkleRoot);
  using hash_t = decltype(Block::previousBlock);

  Miner(std::shared_ptr<IBlockchain<Block>> chain,
        std::shared_ptr<ChainParams> params)
      : blockchain_(std::move(chain)), params_(std::move(params)) {}

  void bootstrap() {
    ValidationState state;
    if (!blockchain_->bootstrap(0, params_->getGenesisBlock(), state)) {
      throw std::logic_error(
          format("miner bootstrapped with invalid block: %s, %s",
                 state.GetRejectReason(),
                 state.GetDebugMessage()));
    }
  }

  Block mineNextBlock(const merkle_t& merkle = merkle_t{}) {
    Block block = getBlockTemplate(merkle);
    createBlock(block);
    ValidationState state;
    if (!blockchain_->acceptBlock(block, state)) {
      throw std::logic_error(format("miner created invalid block: %s, %s",
                                    state.GetRejectReason(),
                                    state.GetDebugMessage()));
    }
    return block;
  }

  // One must define their own template specialization for given Block and
  // ChainParams types. Otherwise, get pretty compilation error.
  Block getBlockTemplate(const merkle_t& /*ignore*/) const {
    static_assert(sizeof(Block) == 0,
                  "There is no template definition for these types. Use "
                  "existing definitions or define them yourself.");
  }

 private:
  std::shared_ptr<IBlockchain<Block>> blockchain_;
  std::shared_ptr<ChainParams> params_;
};

// TODO: move this to vbk_miner.hpp and implement proper method
// template <>
// VbkBlock Miner<VbkBlock, VbkChainParams>::getBlockTemplate(
//    const merkle_t& merkle) {
//  VbkBlock block;
//
//  // TODO: figure out better way to set this data
//  block.version = blocks_[0].version;
//  block.difficulty = blocks_[0].difficulty;
//  block.previousBlock = blocks_[blocks_.size() - 1]
//                            .getHash()
//                            .template trim<VBLAKE_PREVIOUS_BLOCK_HASH_SIZE>();
//  if (blocks_.size() >= KEYSTONE_INTERVAL) {
//    size_t lastIndex = blocks_.size() - 1;
//    // set first previous keystone
//    size_t prevDiff = lastIndex % KEYSTONE_INTERVAL;
//    block.previousKeystone =
//        blocks_[lastIndex - prevDiff]
//            .getHash()
//            .template trim<VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE>();
//
//    // set second previous keystone
//    if (blocks_.size() >= 2 * KEYSTONE_INTERVAL) {
//      block.secondPreviousKeystone =
//          blocks_[lastIndex - prevDiff - KEYSTONE_INTERVAL]
//              .getHash()
//              .template trim<VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE>();
//    }
//  }
//  block.timestamp = currentTimestamp4();
//  block.merkleRoot = merkle;
//  return block;
//}

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_MINER_HPP_
