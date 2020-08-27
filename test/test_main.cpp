// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <sstream>
#include <veriblock/fmt.hpp>

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
  int seed = 0;

  if (argc == 2 && argv[1]) {
    str2int(seed, argv[1]);
  }

  srand(seed);
  fmt::printf("[~~~~~~~~~~] Running test suite with seed=%d.\n", seed);

  return RUN_ALL_TESTS();
}