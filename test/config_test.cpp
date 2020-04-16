#include <gtest/gtest.h>

#include <util/alt_chain_params_regtest.hpp>
#include <veriblock/altintegration.hpp>

using namespace altintegration;

std::vector<std::string> parseBlocks(const std::string& csv) {
  std::istringstream iss(csv);

  std::vector<std::string> ret;
  std::string line;
  while (std::getline(iss, line, ',')) {
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