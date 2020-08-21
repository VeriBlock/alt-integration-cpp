// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/rewards/poprewards_calculator.hpp>

using namespace altintegration;

struct AltChainParamsTest : public AltChainParams {
  AltBlock getBootstrapBlock() const noexcept override {
    AltBlock genesisBlock;
    genesisBlock.hash = {1, 2, 3};
    genesisBlock.previousBlock = {4, 5, 6};
    genesisBlock.height = 0;
    genesisBlock.timestamp = 0;
    return genesisBlock;
  }

  int64_t getIdentifier() const noexcept override { return 0x7ec7; }

  std::vector<uint8_t> getHash(
      const std::vector<uint8_t>& bytes) const noexcept override {
    ReadStream stream(bytes);
    AltBlock altBlock = AltBlock::fromVbkEncoding(stream);
    return altBlock.getHash();
  }
};

int main() {
  AltChainParamsTest chainParams{};
  PopRewardsCalculator rewardsCalculator = PopRewardsCalculator(chainParams);
  PopRewardsBigDecimal score = 1.0;
  PopRewardsBigDecimal difficulty = 1.0;
  // pay 20 VBTC for each reward point
  double popCoefficient = 20.0;
  const std::vector<int> minersCount = {1, 2, 5, 10, 25};
  const std::vector<int> endorsementsCount = {1, 2, 5, 10, 25};
  const std::vector<int> difficulties = {1, 2, 5, 10, 25};
  const std::vector<int> heights = {2, 3, 4, 5};

  const auto delimiter = ";";
  const std::vector<std::string> headerParts = {"height",
                                                "round",
                                                "score",
                                                "total_miners",
                                                "total_endorsements",
                                                "difficulty",
                                                "block_reward",
                                                "miner_reward"};
  const std::vector<std::string> formatParts = {
      "%d", "%d", "%d", "%d", "%d", "%d", "%.4f", "%.4f"};

  // do the action

  const std::string format = fmt::format(
      "{}\n", fmt::join(formatParts.begin(), formatParts.end(), delimiter));
  const std::string header = fmt::format(
      "{}", fmt::join(headerParts.begin(), headerParts.end(), delimiter));

  fmt::printf("%s\n", header);
  for (const auto& h : heights) {
    for (const auto& m : minersCount) {
      for (const auto& d : difficulties) {
        for (const auto& e : endorsementsCount) {
          score = e * 1.0;
          difficulty = d * 1.0;
          auto blockReward =
              rewardsCalculator.calculateBlockReward(h, score, difficulty);
          double blockRewardDouble = ((double)blockReward.value.getLow64()) /
                                     PopRewardsBigDecimal::decimals;
          blockRewardDouble *= popCoefficient;
          auto minerReward = blockRewardDouble / m;
          fmt::printf(format,
                      h,
                      rewardsCalculator.getRoundForBlockNumber(h),
                      score.getIntegerFraction(),
                      m,
                      e,
                      difficulty.getIntegerFraction(),
                      blockRewardDouble,
                      minerReward);
        }
      }
    }
  }
}
