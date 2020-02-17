#include <gtest/gtest.h>
#include <stdint.h>

#include <vector>

#include "veriblock/entities/publication_data.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/mock_miner.hpp"
#include "veriblock/stateless_validation.hpp"

using namespace VeriBlock;

struct MockMinerTest : public MockMiner, public ::testing::Test {
  PublicationData publicationData;
  ValidationState state;

  VbkTx generateSignedbkTxTest(const PublicationData& pub) {
    return this->generateSignedVbkTx(pub);
  }

  ATV generateValidATVTest(const PublicationData& pub) {
    return this->generateValidATV(pub);
  }

  std::shared_ptr<VbkChainParams> getVbkParams() const { return vbk_params; }

  std::shared_ptr<BtcChainParams> getBtcParams() const { return btc_params; }

  MockMinerTest() {
    publicationData.contextInfo = std::vector<uint8_t>(100, 1);
    publicationData.header = std::vector<uint8_t>(100, 2);
    publicationData.identifier = 1;
    publicationData.payoutInfo = std::vector<uint8_t>(100, 3);
  }
};

TEST_F(MockMinerTest, generateSignedVbkTx_test) {
  VbkTx validTransaction = generateSignedbkTxTest(publicationData);
  EXPECT_TRUE(checkVbkTx(validTransaction, state));
  EXPECT_TRUE(state.IsValid());
}

TEST_F(MockMinerTest, generateValidATV_test) {
  ATV validATV = generateValidATVTest(publicationData);
  EXPECT_TRUE(checkATV(validATV, state, *getVbkParams()));
  EXPECT_TRUE(state.IsValid());
}
