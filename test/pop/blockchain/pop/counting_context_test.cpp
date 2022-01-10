// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/blockchain/pop/counting_context.hpp>

#include "pop/util/pop_test_fixture.hpp"

using namespace altintegration;

TEST(Counter, Amount) {
  auto P = AltChainParamsRegTest();
  P.mMaxATVsInAltBlock = 1;
  P.mMaxVTBsInAltBlock = 1;
  P.mMaxVbkBlocksInAltBlock = 1;
  auto C = CountingContext(P);

  auto atv = ATV{};
  auto vtb = VTB{};
  auto vbk = VbkBlock{};

  ASSERT_TRUE(C.canFit(atv));
  C.update(atv);
  ASSERT_FALSE(C.canFit(atv));

  ASSERT_TRUE(C.canFit(vtb));
  C.update(vtb);
  ASSERT_FALSE(C.canFit(vtb));

  ASSERT_TRUE(C.canFit(vbk));
  C.update(vbk);
  ASSERT_FALSE(C.canFit(vbk));
}

TEST(Counter, Size) {
  auto P = AltChainParamsRegTest();
  P.mMaxPopDataSize =
      (uint32_t)(PopData{}.estimateSize() + ATV{}.estimateSize());
  auto C = CountingContext(P);

  auto atv = ATV{};
  auto vtb = VTB{};
  auto vbk = VbkBlock{};

  ASSERT_TRUE(C.canFit(atv));
  C.update(atv);
  // next payloads do not because of size
  ASSERT_FALSE(C.canFit(atv));
  ASSERT_FALSE(C.canFit(vtb));
  ASSERT_FALSE(C.canFit(vbk));
}
