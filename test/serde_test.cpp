// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>

#include <veriblock/uint.hpp>
#include <veriblock/serde.hpp>
#include <vector>

using namespace altintegration;

TEST(Serde, ToFromArrayRoundTrip) {
  std::vector<uint256> v{
      uint256::fromHex("01"),
      uint256::fromHex("02"),
      uint256::fromHex("03"),
  };

  WriteStream w;
  writeArrayOf<uint256>(w, v, writeSingleByteLenValue);

  ReadStream r(w.data());
  std::vector<uint256> actual = readArrayOf<uint256>(r, [](ReadStream& R) -> uint256 {
    return readSingleByteLenValue(R);
  });

  ASSERT_EQ(v, actual);
}