// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "veriblock/abfi/bitcoin/serialize.hpp"

TEST(Serialize, sizes) {
  EXPECT_EQ(sizeof(char), GetSerializeSize(char(0), 0));
  EXPECT_EQ(sizeof(int8_t), GetSerializeSize(int8_t(0), 0));
  EXPECT_EQ(sizeof(uint8_t), GetSerializeSize(uint8_t(0), 0));
  EXPECT_EQ(sizeof(int16_t), GetSerializeSize(int16_t(0), 0));
  EXPECT_EQ(sizeof(uint16_t), GetSerializeSize(uint16_t(0), 0));
  EXPECT_EQ(sizeof(int32_t), GetSerializeSize(int32_t(0), 0));
  EXPECT_EQ(sizeof(uint32_t), GetSerializeSize(uint32_t(0), 0));
  EXPECT_EQ(sizeof(int64_t), GetSerializeSize(int64_t(0), 0));
  EXPECT_EQ(sizeof(uint64_t), GetSerializeSize(uint64_t(0), 0));
  EXPECT_EQ(sizeof(float), GetSerializeSize(float(0), 0));
  EXPECT_EQ(sizeof(double), GetSerializeSize(double(0), 0));
  // Bool is serialized as char
  EXPECT_EQ(sizeof(char), GetSerializeSize(bool(0), 0));

  // Sanity-check GetSerializeSize and c++ type matching
  EXPECT_EQ(GetSerializeSize(char(0), 0), 1U);
  EXPECT_EQ(GetSerializeSize(int8_t(0), 0), 1U);
  EXPECT_EQ(GetSerializeSize(uint8_t(0), 0), 1U);
  EXPECT_EQ(GetSerializeSize(int16_t(0), 0), 2U);
  EXPECT_EQ(GetSerializeSize(uint16_t(0), 0), 2U);
  EXPECT_EQ(GetSerializeSize(int32_t(0), 0), 4U);
  EXPECT_EQ(GetSerializeSize(uint32_t(0), 0), 4U);
  EXPECT_EQ(GetSerializeSize(int64_t(0), 0), 8U);
  EXPECT_EQ(GetSerializeSize(uint64_t(0), 0), 8U);
  EXPECT_EQ(GetSerializeSize(float(0), 0), 4U);
  EXPECT_EQ(GetSerializeSize(double(0), 0), 8U);
  EXPECT_EQ(GetSerializeSize(bool(0), 0), 1U);
}

TEST(Serialize, floats_conversion) {
  // Choose values that map unambiguously to binary floating point to avoid
  // rounding issues at the compiler side.
  EXPECT_EQ(ser_uint32_to_float(0x00000000), 0.0F);
  EXPECT_EQ(ser_uint32_to_float(0x3f000000), 0.5F);
  EXPECT_EQ(ser_uint32_to_float(0x3f800000), 1.0F);
  EXPECT_EQ(ser_uint32_to_float(0x40000000), 2.0F);
  EXPECT_EQ(ser_uint32_to_float(0x40800000), 4.0F);
  EXPECT_EQ(ser_uint32_to_float(0x44444444), 785.066650390625F);

  EXPECT_EQ(ser_float_to_uint32(0.0F), 0x00000000U);
  EXPECT_EQ(ser_float_to_uint32(0.5F), 0x3f000000U);
  EXPECT_EQ(ser_float_to_uint32(1.0F), 0x3f800000U);
  EXPECT_EQ(ser_float_to_uint32(2.0F), 0x40000000U);
  EXPECT_EQ(ser_float_to_uint32(4.0F), 0x40800000U);
  EXPECT_EQ(ser_float_to_uint32(785.066650390625F), 0x44444444U);
}
