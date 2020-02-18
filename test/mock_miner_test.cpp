#include <gtest/gtest.h>
#include <stdint.h>

#include <vector>

#include "veriblock/entities/publication_data.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/mock_miner.hpp"
#include "veriblock/stateless_validation.hpp"
#include "veriblock/storage/block_repository_inmem.hpp"

using namespace VeriBlock;

struct MockMinerTest : public MockMiner, public ::testing::Test {
  PublicationData publicationData;
  ValidationState state;

  MockMinerTest()
      : MockMiner(std::make_shared<BlockRepositoryInmem<btc_block_index_t>>(),
                  std::make_shared<BlockRepositoryInmem<vbk_block_index_t>>()) {
    publicationData.contextInfo = std::vector<uint8_t>(100, 1);
    publicationData.header = std::vector<uint8_t>(100, 2);
    publicationData.identifier = 1;
    publicationData.payoutInfo = std::vector<uint8_t>(100, 3);
  }
};

TEST_F(MockMinerTest, mine_test) {
  Publications pubs = mine(publicationData,
                           getVbkParams()->getGenesisBlock().getHash(),
                           getBtcParams()->getGenesisBlock().getHash(),
                           5);

  EXPECT_TRUE(checkATV(pubs.atv, state, *getVbkParams()));
  EXPECT_TRUE(state.IsValid());

  for (const auto& vtb : pubs.vtbs) {
    EXPECT_TRUE(checkVTB(vtb, state, *getVbkParams(), *getBtcParams()));
    EXPECT_TRUE(state.IsValid());
  }
}
