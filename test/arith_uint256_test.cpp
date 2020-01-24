#include <gtest/gtest.h>

#include <algorithm>

#include "util/literals.hpp"
#include "veriblock/arith_uint256.hpp"

using namespace VeriBlock;

const unsigned char R1Array[] =
    "\x9c\x52\x4a\xdb\xcf\x56\x11\x12\x2b\x29\x12\x5e\x5d\x35\xd2\xd2"
    "\x22\x81\xaa\xb5\x33\xf0\x08\x32\xd5\x56\xb1\xf9\xea\xe5\x1d\x7d";
const double R1Ldouble =
    0.4887374590559308955;  // R1L equals roughly R1Ldouble * 2^256
const ArithUint256 R1L(std::vector<uint8_t>(R1Array,
                                            R1Array + SHA256_HASH_SIZE));
const uint64_t R1LLow64 = 0x121156cfdb4a529cULL;

const unsigned char R2Array[] =
    "\x70\x32\x1d\x7c\x47\xa5\x6b\x40\x26\x7e\x0a\xc3\xa6\x9c\xb6\xbf"
    "\x13\x30\x47\xa3\x19\x2d\xda\x71\x49\x13\x72\xf0\xb4\xca\x81\xd7";
const ArithUint256 R2L(std::vector<uint8_t>(R2Array,
                                            R2Array + SHA256_HASH_SIZE));

const std::vector<uint8_t> R1PlusR2LVec =
    "549FB09FEA236A1EA3E31D4D58F1B1369288D204211CA751527CFC175767850C"_unhex;
const ArithUint256 R1LPlusR2L(std::vector<uint8_t>(R1PlusR2LVec.rbegin(),
                                                   R1PlusR2LVec.rend()));

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
  ArithUint256 c1L = ArithUint256(0x0123456789abcdefULL);
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
  std::vector<uint8_t> bytes =
      "62a38c0486f01e45879d7910a7761bf30d5237e9873f9bff3642a732c4d84f10"_unhex;
  ASSERT_TRUE((R1L * R1L) ==
              ArithUint256(std::vector<uint8_t>(bytes.rbegin(), bytes.rend())));
  bytes =
      "de37805e9986996cfba76ff6ba51c008df851987d9dd323f0e5de07760529c40"_unhex;
  ASSERT_TRUE((R1L * R2L) ==
              ArithUint256(std::vector<uint8_t>(bytes.rbegin(), bytes.rend())));
  ASSERT_TRUE((R1L * ZeroL) == ZeroL);
  ASSERT_TRUE((R1L * OneL) == R1L);
  ASSERT_TRUE((R1L * MaxL) == -R1L);
  ASSERT_TRUE((R2L * R1L) == (R1L * R2L));
  bytes =
      "ac8c010096767d3cae5005dec28bb2b45a1d85ab7996ccd3e102a650f74ff100"_unhex;
  ASSERT_TRUE((R2L * R2L) ==
              ArithUint256(std::vector<uint8_t>(bytes.rbegin(), bytes.rend())));
  ASSERT_TRUE((R2L * ZeroL) == ZeroL);
  ASSERT_TRUE((R2L * OneL) == R2L);
  ASSERT_TRUE((R2L * MaxL) == -R2L);

  ASSERT_TRUE(MaxL * MaxL == OneL);

  ASSERT_TRUE((R1L * 0) == ArithUint256(0));
  ASSERT_TRUE((R1L * 1) == R1L);

  bytes =
      "7759b1c0ed14047f961ad09b20ff83687876a0181a367b813634046f91def7d4"_unhex;
  ASSERT_TRUE((R1L * 3) ==
              ArithUint256(std::vector<uint8_t>(bytes.rbegin(), bytes.rend())));

  bytes =
      "23f7816e30c4ae2017257b7a0fa64d60402f5234d46e746b61c960d09a26d070"_unhex;
  ASSERT_TRUE((R2L * 0x87654321UL) ==
              ArithUint256(std::vector<uint8_t>(bytes.rbegin(), bytes.rend())));
}

TEST(ArithUint256, divide) {
  std::vector<uint8_t> bytes =
      "00000000000000000000000000000000000000000000000ad7133ac1977fa2b7"_unhex;
  ArithUint256 D1L(std::vector<uint8_t>(bytes.rbegin(), bytes.rend()));

  bytes =
      "0000000000000000000000000000000000000000000000000000000ecd751716"_unhex;
  ArithUint256 D2L(std::vector<uint8_t>(bytes.rbegin(), bytes.rend()));

  bytes =
      "00000000000000000b8ac01106981635d9ed112290f8895545a7654dde28fb3a"_unhex;
  ASSERT_TRUE(R1L / D1L ==
              ArithUint256(std::vector<uint8_t>(bytes.rbegin(), bytes.rend())));

  bytes =
      "000000000873ce8efec5b67150bad3aa8c5fcb70e947586153bf2cec7c37c57a"_unhex;
  ASSERT_TRUE(R1L / D2L ==
              ArithUint256(std::vector<uint8_t>(bytes.rbegin(), bytes.rend())));
  ASSERT_TRUE(R1L / OneL == R1L);
  ASSERT_TRUE(R1L / MaxL == ZeroL);
  ASSERT_TRUE(MaxL / R1L == ArithUint256(2));
  ASSERT_THROW(R1L / ZeroL, uint_error);

  bytes =
      "000000000873ce8efec5b67150bad3aa8c5fcb70e947586153bf2cec7c37c57a"_unhex;
  ASSERT_TRUE(R2L / D1L ==
              ArithUint256(std::vector<uint8_t>(bytes.rbegin(), bytes.rend())));
  bytes =
      "000000000873ce8efec5b67150bad3aa8c5fcb70e947586153bf2cec7c37c57a"_unhex;
  ASSERT_TRUE(R2L / D2L ==
              ArithUint256(std::vector<uint8_t>(bytes.rbegin(), bytes.rend())));
  ASSERT_TRUE(R2L / OneL == R2L);
  ASSERT_TRUE(R2L / MaxL == ZeroL);
  ASSERT_TRUE(MaxL / R2L == ArithUint256(1));
}
