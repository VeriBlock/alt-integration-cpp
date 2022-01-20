// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <sstream>
#include <pop/util/fmtlogger.hpp>
#include <veriblock/pop/fmt.hpp>
#include <veriblock/pop/logger.hpp>

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

  LogLevel level = LogLevel::off;

  int seed = 0;

  if (argc >= 2 && argv[1] != nullptr) {
    str2int(seed, argv[1]);
  }

  if (argc >= 3 && argv[2] != nullptr) {
    level = StringToLevel(argv[2]);
  }

  srand(seed);
  SetLogger<FmtLogger>(level);
  fmt::printf("[~~~~~~~~~~] Seed=%d\n", seed);
  fmt::printf("[~~~~~~~~~~] LogLevel=%s\n", LevelToString(level));

  return RUN_ALL_TESTS();
}