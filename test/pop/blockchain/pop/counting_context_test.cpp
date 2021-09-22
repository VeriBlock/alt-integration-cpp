// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/blockchain/pop/counting_context.hpp>

#include "util/pop_test_fixture.hpp"

using namespace altintegration;

TEST(Counter, Amount) {
  auto P = AltChainParamsTest();
  P.mMaxATVsInAltBlock = 1;
  P.mMaxVTBsInAltBlock = 1;
  P.mMaxVbkBlocksInAltBlock = 1;
  auto C = CountingContext(P);

  ASSERT_TRUE(C.canFit(ATV{}));
  ASSERT_FALSE(C.canFit(ATV{}));

  ASSERT_TRUE(C.canFit(VTB{}));
  ASSERT_FALSE(C.canFit(VTB{}));

  ASSERT_TRUE(C.canFit(VbkBlock{}));
  ASSERT_FALSE(C.canFit(VbkBlock{}));
}

TEST(Counter, Size) {
  auto P = AltChainParamsTest();
  P.mMaxPopDataSize = (uint32_t)(PopData{}.estimateSize() + ATV{}.estimateSize());
  auto C = CountingContext(P);

  ASSERT_TRUE(C.canFit(ATV{}));
  // next payloads do not because of size
  ASSERT_FALSE(C.canFit(ATV{}));
  ASSERT_FALSE(C.canFit(VTB{}));
  ASSERT_FALSE(C.canFit(VbkBlock{}));
}
