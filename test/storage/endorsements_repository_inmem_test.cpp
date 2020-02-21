#include <gtest/gtest.h>
#include <stdint.h>

#include <memory>
#include <random>
#include <vector>

#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbkpoptx.hpp"
#include "veriblock/storage/endorsements_repository_inmem.hpp"

using namespace VeriBlock;

struct EndorsementRepoTestCase {
  std::shared_ptr<EndorsementsRepository> endorsement_repo;
};

struct EndorsementRepoTest
    : public ::testing::TestWithParam<EndorsementRepoTestCase> {
  EndorsementRepoTest() { srand(0); }

  VbkBlock generateRandomVbkBlock() {
    VbkBlock block;

    block.difficulty = (int32_t)rand();
    block.height = (int32_t)rand();
    block.nonce = (int32_t)rand();
    block.timestamp = (int32_t)rand();
    block.version = (int16_t)rand();

    return block;
  }

  BtcBlock generateRandomBtcBlock() {
    BtcBlock block;

    block.bits = (uint32_t)rand();
    block.nonce = (uint32_t)rand();
    block.timestamp = (uint32_t)rand();
    block.version = (uint32_t)rand();

    return block;
  }

  VbkPopTx generateVbkPopTx(const std::vector<uint8_t>& btcTxBytes,
                            const std::vector<uint8_t>& address,
                            const VbkBlock& endorsedBlock,
                            const BtcBlock& blockOfProof) {
    VbkPopTx popTx;
    popTx.bitcoinTransaction = BtcTx(btcTxBytes);
    popTx.publishedBlock = endorsedBlock;
    popTx.address = Address::fromPublicKey(address);
    popTx.blockOfProof = blockOfProof;

    return popTx;
  }
};

static std::vector<EndorsementRepoTestCase> endorsement_repos = {
    {std::make_shared<EndorsementsRepositoryInmem>()}};

TEST_P(EndorsementRepoTest, basic_test) {
  auto value = GetParam();
  auto repo = value.endorsement_repo;

  VbkBlock containingBlock = generateRandomVbkBlock();
  VbkBlock endorsedBlock = generateRandomVbkBlock();
  BtcBlock blockOfProof = generateRandomBtcBlock();

  VbkPopTx popTx =
      generateVbkPopTx({1, 2, 3}, {1, 2, 3}, endorsedBlock, blockOfProof);

  repo->put(popTx, containingBlock.getHash());

  std::vector<Endorsement> endorsements = repo->getEndorsementsInChain(
      endorsedBlock.getHash(),
      [&containingBlock](const VbkBlock::hash_t& hash) -> bool {
        return hash == containingBlock.getHash();
      });

  EXPECT_EQ(endorsements.size(), 1);
  EXPECT_EQ(endorsements[0].blockOfProof, blockOfProof.getHash());
  EXPECT_EQ(endorsements[0].endorsementKey, popTx.getHash());
  EXPECT_EQ(endorsements[0].endorsedVbkHash, endorsedBlock.getHash());
  EXPECT_EQ(endorsements[0].containingVbkHashes.size(), 1);
  EXPECT_EQ(endorsements[0].containingVbkHashes[0], containingBlock.getHash());

  VbkPopTx popTx2 =
      generateVbkPopTx({1, 2, 4}, {1, 2, 4}, endorsedBlock, blockOfProof);

  EXPECT_TRUE(popTx2.getHash() != popTx.getHash());

  repo->put(popTx2, containingBlock.getHash());

  endorsements = repo->getEndorsementsInChain(
      endorsedBlock.getHash(),
      [&containingBlock](const VbkBlock::hash_t& hash) -> bool {
        return hash == containingBlock.getHash();
      });

  EXPECT_EQ(endorsements.size(), 2);
  EXPECT_EQ(endorsements[0].blockOfProof, blockOfProof.getHash());
  EXPECT_EQ(endorsements[0].endorsementKey, popTx.getHash());
  EXPECT_EQ(endorsements[0].endorsedVbkHash, endorsedBlock.getHash());
  EXPECT_EQ(endorsements[0].containingVbkHashes.size(), 1);
  EXPECT_EQ(endorsements[0].containingVbkHashes[0], containingBlock.getHash());

  EXPECT_EQ(endorsements.size(), 2);
  EXPECT_EQ(endorsements[1].blockOfProof, blockOfProof.getHash());
  EXPECT_EQ(endorsements[1].endorsementKey, popTx2.getHash());
  EXPECT_EQ(endorsements[1].endorsedVbkHash, endorsedBlock.getHash());
  EXPECT_EQ(endorsements[1].containingVbkHashes.size(), 1);
  EXPECT_EQ(endorsements[1].containingVbkHashes[0], containingBlock.getHash());

  VbkBlock containingBlock2 = generateRandomVbkBlock();

  repo->put(popTx2, containingBlock2.getHash());

  endorsements =
      repo->getEndorsementsInChain(endorsedBlock.getHash(),
                                   [&containingBlock, &containingBlock2](
                                       const VbkBlock::hash_t& hash) -> bool {
                                     return hash == containingBlock.getHash() ||
                                            hash == containingBlock2.getHash();
                                   });

  EXPECT_EQ(endorsements.size(), 2);
  EXPECT_EQ(endorsements[1].blockOfProof, blockOfProof.getHash());
  EXPECT_EQ(endorsements[1].endorsementKey, popTx2.getHash());
  EXPECT_EQ(endorsements[1].endorsedVbkHash, endorsedBlock.getHash());
  EXPECT_EQ(endorsements[1].containingVbkHashes.size(), 2);
  EXPECT_EQ(endorsements[1].containingVbkHashes[1], containingBlock2.getHash());

  repo->clear();

  endorsements =
      repo->getEndorsementsInChain(endorsedBlock.getHash(),
                                   [&containingBlock, &containingBlock2](
                                       const VbkBlock::hash_t& hash) -> bool {
                                     return hash == containingBlock.getHash() ||
                                            hash == containingBlock2.getHash();
                                   });

  EXPECT_EQ(endorsements.size(), 0);
}

INSTANTIATE_TEST_SUITE_P(EndorsementRepoRegression,
                         EndorsementRepoTest,
                         testing::ValuesIn(endorsement_repos));
