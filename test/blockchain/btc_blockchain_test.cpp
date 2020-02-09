#include <gtest/gtest.h>

#include <veriblock/blockchain/blockchain.hpp>

#include "mock/storage/block_repository_mock.hpp"
#include "mock/storage/cursor_mock.hpp"
#include "veriblock/blockchain/btc_miner.hpp"

using namespace VeriBlock;
using ::testing::_;
using ::testing::Field;
using ::testing::Return;
using ::testing::StrictMock;


// eventually this will be type parametrized test
struct BtcBlockchainTest : public ::testing::Test {
  using block_t = BtcBlock;
  using index_t = typename Blockchain<block_t>::index_t;
  using height_t = typename Blockchain<block_t>::height_t;
  using params_t = BtcChainParams;

  std::shared_ptr<StrictMock<BlockRepositoryMock<index_t>>> repo;
  std::shared_ptr<StrictMock<CursorMock<height_t, index_t>>> cursor;
  std::shared_ptr<IBlockchain<block_t>> blockchain;

  std::shared_ptr<params_t> chainparam;
  std::shared_ptr<Miner<block_t, params_t>> miner;

  BtcBlockchainTest() {
    chainparam = std::make_shared<BtcChainParamsRegTest>();

    cursor = std::make_shared<StrictMock<CursorMock<height_t, index_t>>>();
    repo = std::make_shared<StrictMock<BlockRepositoryMock<index_t>>>();
    blockchain = std::make_shared<Blockchain<block_t>>(repo);

    miner = std::make_shared<Miner<block_t, params_t>>(blockchain, chainparam);

    EXPECT_CALL(*repo, getCursor()).WillRepeatedly(Return(cursor));
  };
};

/**
 * Scenario 1
 *
 * @given empty blockchain.
 * @when Bootstrap it with BtcRegTest genesis block at height 0 and mine 10000
 * consecutive blocks.
 * @then stored blockchain is valid:
 * - every block has previousHash set correctly
 * - all blocks have same difficulty
 * - timestamp for blocks does not decrease
 * - blocks are statelessly valid
 */
TEST_F(BtcBlockchainTest, Scenario1) {
  // @given
  auto genesis = chainparam->getGenesisBlock();

  EXPECT_CALL(*repo, put(Field(&index_t::header, genesis)))
      .WillRepeatedly(Return(false));
  EXPECT_CALL(*cursor, seekToFirst()).Times(1);
  // database is empty, so first isValid will return false
  EXPECT_CALL(*cursor, isValid()).WillOnce(Return(false));

  // @when
  ValidationState state;
  height_t height = 0;
  auto result = blockchain->bootstrap(height, genesis, state);
  EXPECT_TRUE(result);
  EXPECT_TRUE(state.IsValid());

  auto& chain = blockchain->getBestChain();
  EXPECT_NE(chain.tip(), nullptr);
  EXPECT_EQ(chain.tip()->height, height);
  EXPECT_EQ(chain.size(), height + 1);
  EXPECT_NE(chain[height], nullptr);
  EXPECT_EQ(chain[height]->height, height);
  EXPECT_EQ(chain[height]->header, genesis);

  EXPECT_CALL(*repo, put(_)).WillRepeatedly(Return(false));

  // mine 10000 blocks
  std::vector<BtcBlock> blocks{genesis};
  std::generate_n(std::back_inserter(blocks), 10000, [&]() {
    return miner->mineNextBlock({});
  });

  // @then
  for (size_t i = 1; i < blocks.size(); i++) {
    // corrent previousBlock set
    EXPECT_EQ(blocks[i].previousBlock, blocks[i - 1].getHash());
    // timestamp is increasing
    EXPECT_GE(blocks[i].timestamp, blocks[i - 1].timestamp);
    // bits is same for RegTest
    EXPECT_EQ(blocks[i].getDifficulty(), blocks[i - 1].getDifficulty())
        << "different at " << i;
    // block is statelessly valid
    EXPECT_TRUE(checkBlock(blocks[i], state));
  }
}
