#include "veriblock/mock_miner.hpp"

#include <gtest/gtest.h>
#include <stdint.h>

#include <vector>

#include "veriblock/entities/publication_data.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/stateless_validation.hpp"

using namespace AltIntegrationLib;

struct MockMinerTest : public MockMiner, public ::testing::Test {
  PublicationData publicationData;
  ValidationState state;

  MockMinerTest() {
    publicationData.contextInfo = std::vector<uint8_t>(100, 1);
    publicationData.header = std::vector<uint8_t>(100, 2);
    publicationData.identifier = 1;
    publicationData.payoutInfo = std::vector<uint8_t>(100, 3);

    EXPECT_TRUE(btc().bootstrapWithGenesis(state));
    EXPECT_TRUE(state.IsValid()) << state.GetRejectReason();
    EXPECT_TRUE(vbk().bootstrapWithGenesis(state));
    EXPECT_TRUE(state.IsValid()) << state.GetRejectReason();
  }
};

TEST_F(MockMinerTest, mine_test) {
  Publications pubs = mine(publicationData,
                           getVbkParams()->getGenesisBlock().getHash(),
                           getBtcParams()->getGenesisBlock().getHash(),
                           5,
                           state);
  ASSERT_TRUE(state.IsValid()) << state.GetRejectReason();
  EXPECT_TRUE(checkATV(pubs.atv, state, *getVbkParams()));
  EXPECT_TRUE(state.IsValid()) << state.GetRejectReason();

  for (const auto& vtb : pubs.vtbs) {
    EXPECT_TRUE(checkVTB(vtb, state, *getVbkParams(), *getBtcParams()));
    EXPECT_TRUE(state.IsValid());
  }
}
