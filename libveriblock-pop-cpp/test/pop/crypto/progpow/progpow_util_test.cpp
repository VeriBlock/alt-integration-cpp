// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>

#include <veriblock/pop/crypto/progpow.hpp>
#include <veriblock/pop/crypto/progpow/kiss99.hpp>
#include <veriblock/pop/literals.hpp>
#include <veriblock/pop/uint.hpp>

using namespace altintegration;

TEST(ProgPowUtil, BlockHeaderHash) {
  auto header =
      "000F42400002449C60619294546AD825AF03B0935637860679DDD55EE4FD21082E18686E"
      "26BBFDA7D5E4462EF24AE02D67E47D785C9B90F3010100000000000005"_unhex;

  uint256 expected = uint256::assertFromHex(
      "B7E83EDA0F8C02DC941C577D6D5E1F5F764FF00F2E19F5E1FA2CC79B9143773A");
  uint256 actual = progpow::getVbkHeaderHash(header);
  ASSERT_EQ(expected, actual);
}

TEST(ProgPowUtil, FillMix) {
  const int PROGPOW_REGS = 32;
  std::array<uint32_t, PROGPOW_REGS> expected = {
      3900479518, 1726670613, 708725536,  331716320,  2781348795, 3286219950,
      2959636021, 2347338745, 2466866814, 1178259958, 1170596716, 2908752503,
      3638729007, 3298094303, 310988884,  2062242187, 902361097,  2945111516,
      975899115,  3196830480, 2642050012, 1982582084, 1018049329, 703144069,
      1892448091, 911497722,  4051583338, 154854582,  680076310,  3981014871,
      2821196981, 3728806798};
  std::array<uint32_t, PROGPOW_REGS> mix_line;
  progpow::fill_mix(0, 0, mix_line);
  ASSERT_EQ(mix_line, expected);
}

TEST(ProgPowUtil, Kiss99) {
  kiss99_t state;
  state.z = 39;
  state.w = 111111;
  state.jsr = 2222;
  state.jcong = 3333333;

  uint32_t ret = kiss99(state);
  ASSERT_EQ(ret, 3842686391ULL);
}

TEST(ProgPowUtil, Merge) {
  uint32_t expected = 1026036;
  uint32_t actual = progpow::merge(31233, 885, 12836127);
  ASSERT_EQ(expected, actual);
}

TEST(ProgPowUtil, Init) {
  const int PROGPOW_REGS = 32;

  const uint64_t seed = 1;
  const std::vector<int> esrc{
      7,  21, 29, 30, 19, 8,  25, 23, 24, 9,  13, 4, 1,  14, 3,  6,
      12, 18, 17, 27, 2,  15, 10, 20, 5,  28, 16, 0, 31, 11, 26, 22,
  };
  const std::vector<int> edst{
      6,  31, 14, 17, 12, 4,  16, 27, 8, 10, 13, 21, 25, 28, 7, 19,
      15, 23, 11, 5,  18, 26, 0,  30, 2, 20, 24, 22, 29, 3,  9, 1,
  };
  const kiss99_t ernd{1188517988, 993223094, 158895826, 713550867};

  std::vector<int> src(PROGPOW_REGS, 0);
  std::vector<int> dst(PROGPOW_REGS, 0);
  kiss99_t rnd = progpow::progPowInit(seed, src, dst);
  ASSERT_EQ(rnd, ernd);
  ASSERT_EQ(src, esrc);
  ASSERT_EQ(dst, edst);
}
