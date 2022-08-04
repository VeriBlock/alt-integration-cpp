// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <cstddef>

#include "veriblock/pop/blob.hpp"
#include "veriblock/pop/consts.hpp"
#include "veriblock/pop/crypto/progpow/ethash.hpp"

using namespace altintegration;
using namespace progpow;

int main() {
  for (int i = 0; i < 50; i += 8000) {
    auto seed = ethash_calculate_seedhash(i);
    fmt::printf("{{");
    for (size_t j = 0; j < seed.size(); j++) {
      fmt::printf("0x%x,", seed[j]);
    }
    fmt::printf("}},\n");
  }
}
