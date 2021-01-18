// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <sstream>
#include <util/fmtlogger.hpp>
#include <veriblock/fmt.hpp>
#include <veriblock/logger.hpp>

static bool str2int(int& i, char const* s) {
  std::stringstream ss(s);
  ss >> i;
  if (ss.fail()) {
    // not an integer
    return false;
  }
  return true;
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  using namespace altintegration;
  SetLogger<FmtLogger>();

  int seed = 0;
  LogLevel level = LogLevel::off;

  if (argc >= 2 && argv[1] != nullptr) {
    str2int(seed, argv[1]);
  }

  if (argc >= 3 && argv[2] != nullptr) {
    level = StringToLevel(argv[2]);
  }

  srand(seed);
  GetLogger().level = level;
  fmt::printf("[~~~~~~~~~~] Seed=%d\n", seed);
  fmt::printf("[~~~~~~~~~~] LogLevel=%s\n", LevelToString(level));

  return RUN_ALL_TESTS();
}