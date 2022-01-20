// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/arith_uint256.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <ostream>
#include <string>

#include <veriblock/pop/literals.hpp>
#include <veriblock/pop/uint.hpp>

using namespace altintegration;

const unsigned char R1Array[] =
    "\x9c\x52\x4a\xdb\xcf\x56\x11\x12\x2b\x29\x12\x5e\x5d\x35\xd2\xd2"
    "\x22\x81\xaa\xb5\x33\xf0\x08\x32\xd5\x56\xb1\xf9\xea\xe5\x1d\x7d";
const char R1ArrayHex[] =
    "7D1DE5EAF9B156D53208F033B5AA8122D2d2355d5e12292b121156cfdb4a529c";
// const double R1Ldouble =
//    0.4887374590559308955;  // R1L equals roughly R1Ldouble * 2^256
const ArithUint256 R1L =
    uint256(std::vector<unsigned char>(R1Array, R1Array + 32));
const uint64_t R1LLow64 = 0x121156cfdb4a529cULL;

const unsigned char R2Array[] =
    "\x70\x32\x1d\x7c\x47\xa5\x6b\x40\x26\x7e\x0a\xc3\xa6\x9c\xb6\xbf"
    "\x13\x30\x47\xa3\x19\x2d\xda\x71\x49\x13\x72\xf0\xb4\xca\x81\xd7";
const ArithUint256 R2L(std::vector<uint8_t>(R2Array,
                                            R2Array + SHA256_HASH_SIZE));

const ArithUint256 R1LPlusR2L = ArithUint256::fromHex(
    "549FB09FEA236A1EA3E31D4D58F1B1369288D204211CA751527CFC175767850C");

const unsigned char ZeroArray[] =
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

const ArithUint256 ZeroL(std::vector<uint8_t>(ZeroArray,
                                              ZeroArray + SHA256_HASH_SIZE));

const unsigned char OneArray[] =
    "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

const ArithUint256 OneL(std::vector<uint8_t>(OneArray,
                                             OneArray + SHA256_HASH_SIZE));

const unsigned char MaxArray[] =
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";

const ArithUint256 MaxL(std::vector<uint8_t>(MaxArray,
                                             MaxArray + SHA256_HASH_SIZE));

const ArithUint256 HalfL = (OneL << 255);

TEST(ArithUint256, basics) {
  // == and !=
  ASSERT_TRUE(R1L != R2L);
  ASSERT_TRUE(ZeroL != OneL);
  ASSERT_TRUE(OneL != ZeroL);
  ASSERT_TRUE(MaxL != ZeroL);
  ASSERT_TRUE(~MaxL == ZeroL);
  ASSERT_TRUE(((R1L ^ R2L) ^ R1L) == R2L);

  for (unsigned int i = 0; i < 256; ++i) {
    ASSERT_TRUE(ZeroL != (OneL << i));
    ASSERT_TRUE((OneL << i) != ZeroL);
    ASSERT_TRUE(R1L != (R1L ^ (OneL << i)));
  }

  ArithUint256 checkToString = ArithUint256(100);
  ASSERT_EQ(checkToString.toString(), "100");

  ArithUint256 checkFromString = ArithUint256::fromString("100");
  ASSERT_EQ(checkFromString, checkToString);
}

static void shiftArrayRight(unsigned char *to,
                            const unsigned char *from,
                            unsigned int arrayLength,
                            unsigned int bitsToShift) {
  for (unsigned int T = 0; T < arrayLength; ++T) {
    unsigned int F = (T + bitsToShift / 8);
    if (F < arrayLength)
      to[T] = from[F] >> (bitsToShift % 8);
    else
      to[T] = 0;
    if (F + 1 < arrayLength) to[T] |= from[(F + 1)] << (8 - bitsToShift % 8);
  }
}

static void shiftArrayLeft(unsigned char *to,
                           const unsigned char *from,
                           unsigned int arrayLength,
                           unsigned int bitsToShift) {
  for (unsigned int T = 0; T < arrayLength; ++T) {
    if (T >= bitsToShift / 8) {
      unsigned int F = T - bitsToShift / 8;
      to[T] = from[F] << (bitsToShift % 8);
      if (T >= bitsToShift / 8 + 1)
        to[T] |= from[F - 1] >> (8 - bitsToShift % 8);
    } else {
      to[T] = 0;
    }
  }
}

TEST(ArithUint256, shifts) {
  // "<<"  ">>"  "<<="  ">>="
  unsigned char TmpArray[32];
  ArithUint256 TmpL;
  for (unsigned int i = 0; i < 256; ++i) {
    shiftArrayLeft(TmpArray, OneArray, 32, i);
    ASSERT_TRUE(ArithUint256(std::vector<unsigned char>(
                    TmpArray, TmpArray + 32)) == (OneL << i));
    TmpL = OneL;
    TmpL <<= i;
    ASSERT_TRUE(TmpL == (OneL << i));
    ASSERT_TRUE((HalfL >> (255 - i)) == (OneL << i));
    TmpL = HalfL;
    TmpL >>= (255 - i);
    ASSERT_TRUE(TmpL == (OneL << i));

    shiftArrayLeft(TmpArray, R1Array, 32, i);
    ASSERT_TRUE(ArithUint256(std::vector<unsigned char>(
                    TmpArray, TmpArray + 32)) == (R1L << i));
    TmpL = R1L;
    TmpL <<= i;
    ASSERT_TRUE(TmpL == (R1L << i));

    shiftArrayRight(TmpArray, R1Array, 32, i);
    ASSERT_TRUE(ArithUint256(std::vector<unsigned char>(
                    TmpArray, TmpArray + 32)) == (R1L >> i));
    TmpL = R1L;
    TmpL >>= i;
    ASSERT_TRUE(TmpL == (R1L >> i));

    shiftArrayLeft(TmpArray, MaxArray, 32, i);
    ASSERT_TRUE(ArithUint256(std::vector<unsigned char>(
                    TmpArray, TmpArray + 32)) == (MaxL << i));
    TmpL = MaxL;
    TmpL <<= i;
    ASSERT_TRUE(TmpL == (MaxL << i));

    shiftArrayRight(TmpArray, MaxArray, 32, i);
    ASSERT_TRUE(ArithUint256(std::vector<unsigned char>(
                    TmpArray, TmpArray + 32)) == (MaxL >> i));
    TmpL = MaxL;
    TmpL >>= i;
    ASSERT_TRUE(TmpL == (MaxL >> i));
  }
  ArithUint256 c1L = 0x0123456789abcdefULL;
  ArithUint256 c2L = c1L << 128;
  for (unsigned int i = 0; i < 128; ++i) {
    ASSERT_TRUE((c1L << i) == (c2L >> (128 - i)));
  }
  for (unsigned int i = 128; i < 256; ++i) {
    ASSERT_TRUE((c1L << i) == (c2L << (i - 128)));
  }
}

TEST(ArithUint256, unaryOperators) {
  ASSERT_TRUE(~ZeroL == MaxL);

  unsigned char TmpArray[32];
  for (unsigned int i = 0; i < 32; ++i) {
    TmpArray[i] = ~R1Array[i];
  }
  ASSERT_TRUE(ArithUint256(std::vector<unsigned char>(
                  TmpArray, TmpArray + 32)) == (~R1L));

  ASSERT_TRUE(-ZeroL == ZeroL);
  ASSERT_TRUE(-R1L == (~R1L) + 1);
  for (unsigned int i = 0; i < 256; ++i) {
    ASSERT_TRUE(-(OneL << i) == (MaxL << i));
  }
}

// Check if doing _A_ _OP_ _B_ results in the same as applying _OP_ onto each
// element of Aarray and Barray, and then converting the result into an
// arith_uint256.
#define CHECKBITWISEOPERATOR(_A_, _B_, _OP_)           \
  for (unsigned int i = 0; i < 32; ++i) {              \
    TmpArray[i] = _A_##Array[i] _OP_ _B_##Array[i];    \
  }                                                    \
  ASSERT_TRUE(ArithUint256(std::vector<unsigned char>( \
                  TmpArray, TmpArray + 32)) == (_A_##L _OP_ _B_##L));

#define CHECKASSIGNMENTOPERATOR(_A_, _B_, _OP_) \
  TmpL = _A_##L;                                \
  TmpL _OP_## = _B_##L;                         \
  ASSERT_TRUE(TmpL == (_A_##L _OP_ _B_##L));

TEST(ArithUint256, bitwiseOperators) {
  unsigned char TmpArray[32];

  CHECKBITWISEOPERATOR(R1, R2, |)
  CHECKBITWISEOPERATOR(R1, R2, ^)
  CHECKBITWISEOPERATOR(R1, R2, &)
  CHECKBITWISEOPERATOR(R1, Zero, |)
  CHECKBITWISEOPERATOR(R1, Zero, ^)
  CHECKBITWISEOPERATOR(R1, Zero, &)
  CHECKBITWISEOPERATOR(R1, Max, |)
  CHECKBITWISEOPERATOR(R1, Max, ^)
  CHECKBITWISEOPERATOR(R1, Max, &)
  CHECKBITWISEOPERATOR(Zero, R1, |)
  CHECKBITWISEOPERATOR(Zero, R1, ^)
  CHECKBITWISEOPERATOR(Zero, R1, &)
  CHECKBITWISEOPERATOR(Max, R1, |)
  CHECKBITWISEOPERATOR(Max, R1, ^)
  CHECKBITWISEOPERATOR(Max, R1, &)

  ArithUint256 TmpL;
  CHECKASSIGNMENTOPERATOR(R1, R2, |)
  CHECKASSIGNMENTOPERATOR(R1, R2, ^)
  CHECKASSIGNMENTOPERATOR(R1, R2, &)
  CHECKASSIGNMENTOPERATOR(R1, Zero, |)
  CHECKASSIGNMENTOPERATOR(R1, Zero, ^)
  CHECKASSIGNMENTOPERATOR(R1, Zero, &)
  CHECKASSIGNMENTOPERATOR(R1, Max, |)
  CHECKASSIGNMENTOPERATOR(R1, Max, ^)
  CHECKASSIGNMENTOPERATOR(R1, Max, &)
  CHECKASSIGNMENTOPERATOR(Zero, R1, |)
  CHECKASSIGNMENTOPERATOR(Zero, R1, ^)
  CHECKASSIGNMENTOPERATOR(Zero, R1, &)
  CHECKASSIGNMENTOPERATOR(Max, R1, |)
  CHECKASSIGNMENTOPERATOR(Max, R1, ^)
  CHECKASSIGNMENTOPERATOR(Max, R1, &)

  uint64_t Tmp64 = 0xe1db685c9a0b47a2ULL;
  TmpL = R1L;
  TmpL |= Tmp64;
  ASSERT_TRUE(TmpL == (R1L | ArithUint256(Tmp64)));
  TmpL = R1L;
  TmpL |= 0;
  ASSERT_TRUE(TmpL == R1L);
  TmpL ^= 0;
  ASSERT_TRUE(TmpL == R1L);
  TmpL ^= Tmp64;
  ASSERT_TRUE(TmpL == (R1L ^ ArithUint256(Tmp64)));
}

TEST(ArithUint256, comparison) {
  ArithUint256 TmpL;
  for (unsigned int i = 0; i < 256; ++i) {
    TmpL = OneL << i;
    ASSERT_TRUE(TmpL >= ZeroL && TmpL > ZeroL && ZeroL < TmpL && ZeroL <= TmpL);
    ASSERT_TRUE(TmpL >= ArithUint256(0) && TmpL > ArithUint256(0) &&
                ArithUint256(0) < TmpL && ArithUint256(0) <= TmpL);
    TmpL |= R1L;
    ASSERT_TRUE(TmpL >= R1L);
    ASSERT_TRUE((TmpL == R1L) != (TmpL > R1L));
    ASSERT_TRUE((TmpL == R1L) || !(TmpL <= R1L));
    ASSERT_TRUE(R1L <= TmpL);
    ASSERT_TRUE((R1L == TmpL) != (R1L < TmpL));
    ASSERT_TRUE((TmpL == R1L) || !(R1L >= TmpL));
    ASSERT_TRUE(!(TmpL < R1L));
    ASSERT_TRUE(!(R1L > TmpL));
  }
}

TEST(ArithUint256, PlusMinus) {
  ArithUint256 TmpL = 0;

  ASSERT_TRUE(R1L + R2L == R1LPlusR2L);
  TmpL += R1L;
  ASSERT_TRUE(TmpL == R1L);
  TmpL += R2L;
  ASSERT_TRUE(TmpL == R1L + R2L);
  ASSERT_TRUE(OneL + MaxL == ZeroL);
  ASSERT_TRUE(MaxL + OneL == ZeroL);
  for (unsigned int i = 1; i < 256; ++i) {
    ASSERT_TRUE((MaxL >> i) + OneL == (HalfL >> (i - 1)));
    ASSERT_TRUE(OneL + (MaxL >> i) == (HalfL >> (i - 1)));
    TmpL = (MaxL >> i);
    TmpL += OneL;
    ASSERT_TRUE(TmpL == (HalfL >> (i - 1)));
    TmpL = (MaxL >> i);
    TmpL += 1;
    ASSERT_TRUE(TmpL == (HalfL >> (i - 1)));
    TmpL = (MaxL >> i);
    ASSERT_TRUE(TmpL++ == (MaxL >> i));
    ASSERT_TRUE(TmpL == (HalfL >> (i - 1)));
  }

  ASSERT_TRUE(ArithUint256(0xbedc77e27940a7ULL) + 0xee8d836fce66fbULL ==
              ArithUint256(0xbedc77e27940a7ULL + 0xee8d836fce66fbULL));
  TmpL = ArithUint256(0xbedc77e27940a7ULL);
  TmpL += 0xee8d836fce66fbULL;
  ASSERT_TRUE(TmpL == ArithUint256(0xbedc77e27940a7ULL + 0xee8d836fce66fbULL));
  TmpL -= 0xee8d836fce66fbULL;
  ASSERT_TRUE(TmpL == ArithUint256(0xbedc77e27940a7ULL));
  TmpL = R1L;
  ASSERT_TRUE(++TmpL == R1L + 1);

  ASSERT_TRUE(R1L - (-R2L) == R1L + R2L);
  ASSERT_TRUE(R1L - (-OneL) == R1L + OneL);
  ASSERT_TRUE(R1L - OneL == R1L + (-OneL));
  for (unsigned int i = 1; i < 256; ++i) {
    ASSERT_TRUE((MaxL >> i) - (-OneL) == (HalfL >> (i - 1)));
    ASSERT_TRUE((HalfL >> (i - 1)) - OneL == (MaxL >> i));
    TmpL = (HalfL >> (i - 1));
    ASSERT_TRUE(TmpL-- == (HalfL >> (i - 1)));
    ASSERT_TRUE(TmpL == (MaxL >> i));
    TmpL = (HalfL >> (i - 1));
    ASSERT_TRUE(--TmpL == (MaxL >> i));
  }
  TmpL = R1L;
  ASSERT_TRUE(--TmpL == R1L - 1);
}

TEST(ArithUint256, multiply) {
  ASSERT_TRUE(
      (R1L * R1L) ==
      ArithUint256::fromHex(
          "62a38c0486f01e45879d7910a7761bf30d5237e9873f9bff3642a732c4d84f10"));
  ASSERT_TRUE(
      (R1L * R2L) ==
      ArithUint256::fromHex(
          "de37805e9986996cfba76ff6ba51c008df851987d9dd323f0e5de07760529c40"));
  ASSERT_TRUE((R1L * ZeroL) == ZeroL);
  ASSERT_TRUE((R1L * OneL) == R1L);
  ASSERT_TRUE((R1L * MaxL) == -R1L);
  ASSERT_TRUE((R2L * R1L) == (R1L * R2L));

  ASSERT_TRUE(
      (R2L * R2L) ==
      ArithUint256::fromHex(
          "ac8c010096767d3cae5005dec28bb2b45a1d85ab7996ccd3e102a650f74ff100"));
  ASSERT_TRUE((R2L * ZeroL) == ZeroL);
  ASSERT_TRUE((R2L * OneL) == R2L);
  ASSERT_TRUE((R2L * MaxL) == -R2L);

  ASSERT_TRUE(MaxL * MaxL == OneL);

  ASSERT_TRUE((R1L * 0) == ArithUint256(0));
  ASSERT_TRUE((R1L * 1) == R1L);

  ASSERT_TRUE(
      (R1L * 3) ==
      ArithUint256::fromHex(
          "7759b1c0ed14047f961ad09b20ff83687876a0181a367b813634046f91def7d4"));

  ASSERT_TRUE(
      (R2L * 0x87654321UL) ==
      ArithUint256::fromHex(
          "23f7816e30c4ae2017257b7a0fa64d60402f5234d46e746b61c960d09a26d070"));
}

TEST(ArithUint256, divide) {
  auto D1L = ArithUint256::fromHex(
      "00000000000000000000000000000000000000000000000ad7133ac1977fa2b7");

  auto D2L = ArithUint256::fromHex(
      "0000000000000000000000000000000000000000000000000000000ecd751716");

  ASSERT_TRUE(
      R1L / D1L ==
      ArithUint256::fromHex(
          "00000000000000000b8ac01106981635d9ed112290f8895545a7654dde28fb3a"));

  ASSERT_TRUE(
      R1L / D2L ==
      ArithUint256::fromHex(
          "000000000873ce8efec5b67150bad3aa8c5fcb70e947586153bf2cec7c37c57a"));
  ASSERT_TRUE(R1L / OneL == R1L);
  ASSERT_TRUE(R1L / MaxL == ZeroL);
  ASSERT_TRUE(MaxL / R1L == ArithUint256(2));
  ASSERT_THROW(R1L / ZeroL, uint_error);

  ASSERT_TRUE(
      R2L / D1L ==
      ArithUint256::fromHex(
          "000000000000000013e1665895a1cc981de6d93670105a6b3ec3b73141b3a3c5"));

  ASSERT_TRUE(
      R2L / D2L ==
      ArithUint256::fromHex(
          "000000000e8f0abe753bb0afe2e9437ee85d280be60882cf0bd1aaf7fa3cc2c4"));
  ASSERT_TRUE(R2L / OneL == R2L);
  ASSERT_TRUE(R2L / MaxL == ZeroL);
  ASSERT_TRUE(MaxL / R2L == ArithUint256(1));
}

struct TestCase {
  uint32_t bits;
  uint32_t compact;
  std::string hex;
  bool negative = false;
  bool overflow = false;
};

class DecodeBitsTest : public testing::TestWithParam<TestCase> {};

static std::vector<TestCase> decodeBits_cases = {
    {0x04800000,
     0,
     "0000000000000000000000000000000000000000000000000000000000000000",
     false,
     false},
    {0x00123456,
     0,
     "0000000000000000000000000000000000000000000000000000000000000000",
     false,
     false},
    {0x01003456,
     0,
     "0000000000000000000000000000000000000000000000000000000000000000",
     false,
     false},
    {0x02000056,
     0,
     "0000000000000000000000000000000000000000000000000000000000000000",
     false,
     false},
    {0x03000000,
     0,
     "0000000000000000000000000000000000000000000000000000000000000000",
     false,
     false},
    {0x04000000,
     0,
     "0000000000000000000000000000000000000000000000000000000000000000",
     false,
     false},
    {0x00923456,
     0,
     "0000000000000000000000000000000000000000000000000000000000000000",
     false,
     false},
    {0x01803456,
     0,
     "0000000000000000000000000000000000000000000000000000000000000000",
     false,
     false},
    {0x02800056,
     0,
     "0000000000000000000000000000000000000000000000000000000000000000",
     false,
     false},
    {0x03800000,
     0,
     "0000000000000000000000000000000000000000000000000000000000000000",
     false,
     false},
    {0x04800000,
     0,
     "0000000000000000000000000000000000000000000000000000000000000000",
     false,
     false},
    {0x01123456,
     0x01120000U,
     "0000000000000000000000000000000000000000000000000000000000000012",
     false,
     false},
    {0x01fedcba,
     0x01fe0000U,
     "000000000000000000000000000000000000000000000000000000000000007e",
     true,
     false},
    {0x02123456,
     0x02123400U,
     "0000000000000000000000000000000000000000000000000000000000001234",
     false,
     false},
    {0x03123456,
     0x03123456U,
     "0000000000000000000000000000000000000000000000000000000000123456",
     false,
     false},
    {0x04123456,
     0x04123456U,
     "0000000000000000000000000000000000000000000000000000000012345600",
     false,
     false},
    {0x04923456,
     0x04923456U,
     "0000000000000000000000000000000000000000000000000000000012345600",
     true,
     false},
    {0x05009234,
     0x05009234U,
     "0000000000000000000000000000000000000000000000000000000092340000",
     false,
     false},
    {0x20123456,
     0x20123456U,
     "1234560000000000000000000000000000000000000000000000000000000000",
     false,
     false},
};

TEST_P(DecodeBitsTest, uint256_decodeBits) {
  auto value = GetParam();

  auto negative = false;
  auto overflow = false;
  ArithUint256 target =
      ArithUint256::fromBits(value.bits, &negative, &overflow);

  EXPECT_EQ(target.toHex(), value.hex);
  EXPECT_EQ(target.toBits(value.negative), value.compact);
  EXPECT_EQ(negative, value.negative);
  EXPECT_EQ(overflow, value.overflow);
}

TEST(DecodeBitsTest, uint256_overflow) {
  auto negative = false;
  auto overflow = false;
  ArithUint256 target =
      ArithUint256::fromBits(0xff123456, &negative, &overflow);
  (void)target;
  EXPECT_FALSE(negative);
  EXPECT_TRUE(overflow);
}

INSTANTIATE_TEST_SUITE_P(decodeBitsRegression,
                         DecodeBitsTest,
                         testing::ValuesIn(decodeBits_cases));

TEST(DecodeBitsTest, Methods) {
  EXPECT_EQ(R1L, ArithUint256::fromHex(R1ArrayHex));

  ArithUint256 TmpL(R1L);
  EXPECT_EQ(TmpL, R1L);
  TmpL.setHex(R2L.toHex());
  EXPECT_EQ(TmpL, R2L);
  TmpL.setHex(ZeroL.toHex());
  EXPECT_EQ(TmpL, 0);
  TmpL.setHex(HalfL.toHex());
  EXPECT_EQ(TmpL, HalfL);

  TmpL.setHex(R1L.toHex());
  EXPECT_EQ(R1L.size(), 32);
  EXPECT_EQ(R2L.size(), 32);
  EXPECT_EQ(ZeroL.size(), 32);
  EXPECT_EQ(MaxL.size(), 32);
  EXPECT_EQ(R1L.getLow64(), R1LLow64);
  EXPECT_EQ(HalfL.getLow64(), 0x0000000000000000ULL);
  EXPECT_EQ(OneL.getLow64(), 0x0000000000000001ULL);

  std::string hex =
      "7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
  TmpL.setHex(hex);
  EXPECT_EQ(TmpL.toBits(false), 0x207fffffUL);
  EXPECT_EQ(ArithUint256::fromHex(hex).toBits(false), 0x207fffffUL);
}
