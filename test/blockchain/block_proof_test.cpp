#include <gtest/gtest.h>

#include <veriblock/blockchain/btc_blockchain_util.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/miner.hpp>
#include <veriblock/blockchain/vbk_blockchain_util.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/stateless_validation.hpp>
#include <veriblock/storage/block_repository_inmem.hpp>

#include <fstream>

using namespace VeriBlock;

struct BlockProofTest : public ::testing::Test {
  using block_t = VbkBlock;
  using index_t = BlockIndex<block_t>;
  using params_base_t = VbkChainParams;
  using params_t = VbkChainParamsRegTest;

  std::shared_ptr<params_base_t> params;
//  std::shared_ptr<BlockTree<block_t, params_base_t>> tree;
//  std::shared_ptr<Miner<block_t, params_base_t>> miner;
//  std::shared_ptr<BlockRepositoryInmem<index_t>> repo;

  ValidationState state;

  BlockProofTest() {
    params = std::make_shared<params_t>();
//    miner = std::make_shared<Miner<block_t, params_base_t>>(params);
//    repo = std::make_shared<BlockRepositoryInmem<index_t>>();
//    tree = std::make_shared<BlockTree<block_t, params_base_t>>(repo, params);
//    EXPECT_TRUE(tree->bootstrap(0, params->getGenesisBlock(), state));
  }
};

TEST_F(BlockProofTest, A) {
  std::string c = "5113a60099c9f24260476a54";
  std::string a = "5113a60099c9f24260476a546ad38f8a5995053b4b04d16c";
  uint192 b = uint192::fromHex(a);
  std::cout << b.trim<VBLAKE_PREVIOUS_BLOCK_HASH_SIZE>().toHex() << std::endl;
}