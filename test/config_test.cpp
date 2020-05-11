// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/alt_chain_params_regtest.hpp>
#include <veriblock/altintegration.hpp>
#include <veriblock/logger/logger.hpp>
#include <veriblock/logger/logger_default.hpp>

using namespace altintegration;

std::vector<std::string> parseBlocks(const std::string& csv) {
  std::istringstream iss(csv);
  std::vector<std::string> ret;
  std::string line;
  while (std::getline(iss, line, ',')) {
    if(ParseHex(line).empty()) {
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

TEST(Config, TryLogger) {
  NewLogger<LoggerDefault>();
  VBK_LOG_INFO("alt", "hello %s", "world");
  VBK_LOG_WARN("alt", "hello %d", 100);
}
