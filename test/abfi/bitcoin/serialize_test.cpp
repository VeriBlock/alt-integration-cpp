// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "veriblock/abfi/bitcoin/serialize.hpp"
#include "veriblock/pop/hashutil.hpp"
#include "veriblock/pop/literals.hpp"
#include "veriblock/pop/read_stream.hpp"

using namespace btc;
using namespace altintegration;

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

TEST(Serialize, doubles_conversion) {
  // Choose values that map unambiguously to binary floating point to avoid
  // rounding issues at the compiler side.
  EXPECT_EQ(ser_uint64_to_double(0x0000000000000000ULL), 0.0);
  EXPECT_EQ(ser_uint64_to_double(0x3fe0000000000000ULL), 0.5);
  EXPECT_EQ(ser_uint64_to_double(0x3ff0000000000000ULL), 1.0);
  EXPECT_EQ(ser_uint64_to_double(0x4000000000000000ULL), 2.0);
  EXPECT_EQ(ser_uint64_to_double(0x4010000000000000ULL), 4.0);
  EXPECT_EQ(ser_uint64_to_double(0x4088888880000000ULL), 785.066650390625);

  EXPECT_EQ(ser_double_to_uint64(0.0), 0x0000000000000000ULL);
  EXPECT_EQ(ser_double_to_uint64(0.5), 0x3fe0000000000000ULL);
  EXPECT_EQ(ser_double_to_uint64(1.0), 0x3ff0000000000000ULL);
  EXPECT_EQ(ser_double_to_uint64(2.0), 0x4000000000000000ULL);
  EXPECT_EQ(ser_double_to_uint64(4.0), 0x4010000000000000ULL);
  EXPECT_EQ(ser_double_to_uint64(785.066650390625), 0x4088888880000000ULL);
}
/*
Python code to generate the below hashes:

    def reversed_hex(x):
        return binascii.hexlify(''.join(reversed(x)))
    def dsha256(x):
        return hashlib.sha256(hashlib.sha256(x).digest()).digest()

    reversed_hex(dsha256(''.join(struct.pack('<f', x) for x in range(0,1000))))
== '8e8b4cf3e4df8b332057e3e23af42ebc663b61e0495d5e7e32d85099d7f3fe0c'
    reversed_hex(dsha256(''.join(struct.pack('<d', x) for x in range(0,1000))))
== '43d0c82591953c4eafe114590d392676a01585d25b25d433557f0d7878b23f96'
*/
TEST(Serialize, floats) {
  WriteStream writer;
  // encode
  for (int i = 0; i < 1000; i++) {
    SerializeBtc(writer, float(i));
  }

  EXPECT_EQ(
      sha256twice(writer.data()).reverse(),
      uint256::fromHex(
          "8e8b4cf3e4df8b332057e3e23af42ebc663b61e0495d5e7e32d85099d7f3fe0c"));

  ReadStream reader{writer.data()};
  // decode
  for (int i = 0; i < 1000; i++) {
    float j;
    UnserializeBtc(reader, j);
    EXPECT_EQ(j, i);
  }
}

TEST(Serialize, doubles) {
  WriteStream writer;
  // encode
  for (int i = 0; i < 1000; i++) {
    SerializeBtc(writer, double(i));
  }

  EXPECT_EQ(
      sha256twice(writer.data()).reverse(),
      uint256::fromHex(
          "43d0c82591953c4eafe114590d392676a01585d25b25d433557f0d7878b23f96"));

  ReadStream reader{writer.data()};
  // decode
  for (int i = 0; i < 1000; i++) {
    double j;
    UnserializeBtc(reader, j);
    EXPECT_EQ(j, i);
  }
}