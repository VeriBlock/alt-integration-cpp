// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/entities/popdata.hpp>

#include "util/test_utils.hpp"
#include "veriblock/literals.hpp"

using namespace altintegration;

TEST(AltPopTx, DeserializeFromVbkEncoding) {
  ATV atv = AssertDeserializeFromHex<ATV>(defaultAtvEncoded);
  VTB vtb = AssertDeserializeFromHex<VTB>(defaultVtbEncoded);

  PopData expectedPopData = {1, {atv.blockOfProof}, {vtb}, {atv}};
  std::vector<uint8_t> bytes = expectedPopData.toVbkEncoding();

  PopData decodedPopData = AssertDeserializeFromVbkEncoding<PopData>(bytes);
  EXPECT_EQ(decodedPopData, expectedPopData);

  PopData decodedPopDataNew;
  ValidationState state;
  bool ret = DeserializeFromVbkEncoding(bytes, decodedPopDataNew, state);
  ASSERT_TRUE(ret);
  EXPECT_TRUE(state.IsValid());
  EXPECT_EQ(decodedPopDataNew, expectedPopData);
}

TEST(PopData, EstimateSize) {
  ATV atv = AssertDeserializeFromHex<ATV>(defaultAtvEncoded);
  VTB vtb = AssertDeserializeFromHex<VTB>(defaultVtbEncoded);
  VbkBlock vbkBlock = AssertDeserializeFromHex<VbkBlock>(defaultVbkBlockEncoded);

  PopData popData = {1, {vbkBlock}, {vtb}, {atv}};
  EXPECT_EQ(popData.estimateSize(), popData.toVbkEncoding().size());
}
