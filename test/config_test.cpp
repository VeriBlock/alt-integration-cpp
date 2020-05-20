// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/alt_chain_params_regtest.hpp>
#include <veriblock/altintegration.hpp>
#include <veriblock/logger.hpp>

using namespace altintegration;

std::vector<std::string> parseBlocks(const std::string& csv) {
  std::istringstream iss(csv);
  std::vector<std::string> ret;
  std::string line;
  while (std::getline(iss, line, ',')) {
    if (ParseHex(line).empty()) {
      break;
    }
    ret.push_back(line);
  }

  return ret;
}

namespace generated {
extern const char btcblockheaders[];
extern const char vbkblockheaders[];
}  // namespace generated

TEST(Config, IsValid) {
  int popbtcstartheight = 1714177;
  std::string popbtcblocks = generated::btcblockheaders;
  int popvbkstartheight = 430118;
  std::string popvbkblocks = generated::vbkblockheaders;

  altintegration::Config config;
  config.setBTC(popbtcstartheight,
                parseBlocks(popbtcblocks),
                std::make_shared<BtcChainParamsTest>());
  config.setVBK(popvbkstartheight,
                parseBlocks(popvbkblocks),
                std::make_shared<VbkChainParamsTest>());
  config.alt = std::make_shared<AltChainParamsRegTest>();

  ASSERT_NO_THROW(config.validate());
}

TEST(Config, deserialization_test) {
  int popbtcstartheight = 1714177;
  std::string popbtcblocks = generated::btcblockheaders;
  int popvbkstartheight = 430118;
  std::string popvbkblocks = generated::vbkblockheaders;

  altintegration::Config config;
  config.setBTC(popbtcstartheight,
                parseBlocks(popbtcblocks),
                std::make_shared<BtcChainParamsTest>());
  config.setVBK(popvbkstartheight,
                parseBlocks(popvbkblocks),
                std::make_shared<VbkChainParamsTest>());
  config.alt = std::make_shared<AltChainParamsRegTest>();

  std::vector<uint8_t> serialized = config.toRaw();

  Config deserialized = Config::fromRaw(serialized);

  // check bootstrap btc
  EXPECT_EQ(deserialized.btc.blocks.size(), config.btc.blocks.size());
  for (size_t i = 0; i < config.btc.blocks.size(); ++i) {
    EXPECT_EQ(deserialized.btc.blocks[i], config.btc.blocks[i]);
  }
  EXPECT_EQ(deserialized.btc.startHeight, config.btc.startHeight);

  // check bootstrap vbk
  EXPECT_EQ(deserialized.vbk.blocks.size(), config.vbk.blocks.size());
  for (size_t i = 0; i < config.vbk.blocks.size(); ++i) {
    EXPECT_EQ(deserialized.vbk.blocks[i], config.vbk.blocks[i]);
  }
  EXPECT_EQ(deserialized.vbk.startHeight, config.vbk.startHeight);

  // check alt config
  EXPECT_EQ(deserialized.alt->getBootstrapBlock(),
            config.alt->getBootstrapBlock());
  EXPECT_EQ(deserialized.alt->getEndorsementSettlementInterval(),
            config.alt->getEndorsementSettlementInterval());
  EXPECT_EQ(deserialized.alt->getFinalityDelay(),
            config.alt->getFinalityDelay());
  EXPECT_EQ(deserialized.alt->getForkResolutionLookUpTable(),
            config.alt->getForkResolutionLookUpTable());
  EXPECT_EQ(deserialized.alt->getIdentifier(), config.alt->getIdentifier());
  EXPECT_EQ(deserialized.alt->getKeystoneInterval(),
            config.alt->getKeystoneInterval());
  EXPECT_EQ(deserialized.alt->getMaxPopDataPerBlock(),
            config.alt->getMaxPopDataPerBlock());
  EXPECT_EQ(deserialized.alt->getMaxPopDataWeight(),
            config.alt->getMaxPopDataWeight());
  EXPECT_EQ(deserialized.alt->getSuperMaxPopDataWeight(),
            config.alt->getSuperMaxPopDataWeight());

  EXPECT_EQ(deserialized.alt->getRewardParams().difficultyAveragingInterval(),
            config.alt->getRewardParams().difficultyAveragingInterval());
  EXPECT_EQ(deserialized.alt->getRewardParams().flatScoreRound(),
            config.alt->getRewardParams().flatScoreRound());
  EXPECT_EQ(deserialized.alt->getRewardParams().flatScoreRoundUse(),
            config.alt->getRewardParams().flatScoreRoundUse());
  EXPECT_EQ(deserialized.alt->getRewardParams().keystoneRound(),
            config.alt->getRewardParams().keystoneRound());
  EXPECT_EQ(deserialized.alt->getRewardParams().maxScoreThresholdKeystone(),
            config.alt->getRewardParams().maxScoreThresholdKeystone());
  EXPECT_EQ(deserialized.alt->getRewardParams().maxScoreThresholdNormal(),
            config.alt->getRewardParams().maxScoreThresholdNormal());
  EXPECT_EQ(deserialized.alt->getRewardParams().payoutRounds(),
            config.alt->getRewardParams().payoutRounds());
  EXPECT_EQ(deserialized.alt->getRewardParams().relativeScoreLookupTable(),
            config.alt->getRewardParams().relativeScoreLookupTable());
  EXPECT_EQ(deserialized.alt->getRewardParams().rewardSettlementInterval(),
            config.alt->getRewardParams().rewardSettlementInterval());
  EXPECT_EQ(deserialized.alt->getRewardParams().roundRatios(),
            config.alt->getRewardParams().roundRatios());

  EXPECT_EQ(
      deserialized.alt->getRewardParams().getCurveParams().slopeKeystone(),
      config.alt->getRewardParams().getCurveParams().slopeKeystone());
  EXPECT_EQ(deserialized.alt->getRewardParams().getCurveParams().slopeNormal(),
            config.alt->getRewardParams().getCurveParams().slopeNormal());
  EXPECT_EQ(deserialized.alt->getRewardParams().getCurveParams().startOfSlope(),
            config.alt->getRewardParams().getCurveParams().startOfSlope());

  // check btc config
  EXPECT_EQ(deserialized.btc.params->getAllowMinDifficultyBlocks(),
            config.btc.params->getAllowMinDifficultyBlocks());
  EXPECT_EQ(deserialized.btc.params->getDifficultyAdjustmentInterval(),
            config.btc.params->getDifficultyAdjustmentInterval());
  EXPECT_EQ(deserialized.btc.params->getGenesisBlock(),
            config.btc.params->getGenesisBlock());
  EXPECT_EQ(deserialized.btc.params->getPowLimit(),
            config.btc.params->getPowLimit());
  EXPECT_EQ(deserialized.btc.params->getPowNoRetargeting(),
            config.btc.params->getPowNoRetargeting());
  EXPECT_EQ(deserialized.btc.params->getPowTargetSpacing(),
            config.btc.params->getPowTargetSpacing());
  EXPECT_EQ(deserialized.btc.params->getPowTargetTimespan(),
            config.btc.params->getPowTargetTimespan());

  // check vbk config
  EXPECT_EQ(deserialized.vbk.params->getEndorsementSettlementInterval(),
            config.vbk.params->getEndorsementSettlementInterval());
  EXPECT_EQ(deserialized.vbk.params->getFinalityDelay(),
            config.vbk.params->getFinalityDelay());
  EXPECT_EQ(deserialized.vbk.params->getForkResolutionLookUpTable(),
            config.vbk.params->getForkResolutionLookUpTable());
  EXPECT_EQ(deserialized.vbk.params->getGenesisBlock(),
            config.vbk.params->getGenesisBlock());
  EXPECT_EQ(deserialized.vbk.params->getKeystoneInterval(),
            config.vbk.params->getKeystoneInterval());
  EXPECT_EQ(deserialized.vbk.params->getMinimumDifficulty(),
            config.vbk.params->getMinimumDifficulty());
  EXPECT_EQ(deserialized.vbk.params->getPowNoRetargeting(),
            config.vbk.params->getPowNoRetargeting());
  EXPECT_EQ(deserialized.vbk.params->getRetargetPeriod(),
            config.vbk.params->getRetargetPeriod());
  EXPECT_EQ(deserialized.vbk.params->getTargetBlockTime(),
            config.vbk.params->getTargetBlockTime());
  EXPECT_EQ(deserialized.vbk.params->getTransactionMagicByte().hasValue,
            config.vbk.params->getTransactionMagicByte().hasValue);
  EXPECT_EQ(deserialized.vbk.params->getTransactionMagicByte().value,
            config.vbk.params->getTransactionMagicByte().value);
}
