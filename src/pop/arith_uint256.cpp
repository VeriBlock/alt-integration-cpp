// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <algorithm>
#include <array>
#include <veriblock/pop/arith_uint256.hpp>
#include <veriblock/pop/assert.hpp>

#include "veriblock/pop/blob.hpp"
#include "veriblock/pop/consts.hpp"
#include "veriblock/pop/logger.hpp"
#include "veriblock/pop/strutil.hpp"

using namespace altintegration;

ArithUint256 ArithUint256::fromBits(uint32_t bits,
                                    bool* negative,
                                    bool* overflow) {
  ArithUint256 target;
  int nSize = bits >> 24u;
  uint32_t nWord = bits & 0x007fffffu;
  if (nSize <= 3) {
    nWord >>= 8 * (3 - nSize);
    target = nWord;
  } else {
    target = nWord;
    target <<= 8 * (nSize - 3);
  }
  if (negative != nullptr) {
    *negative = (nWord != 0) && ((bits & 0x00800000u) != 0);
  }
  if (overflow != nullptr) {
    *overflow = nWord != 0 && ((nSize > 34) || (nWord > 0xff && nSize > 33) ||
                               (nWord > 0xffff && nSize > 32));
  }

  return target;
}

std::string ArithUint256::toString() const {
  ArithUint256 tmp = ArithUint256(*this);
  std::string out = "";
  while (true) {
    ArithUint256 tmpNext = tmp / 10;
    ArithUint256 decimal = tmp - (tmpNext * 10);
    tmp = tmpNext;
    out.push_back((char)(decimal.getLow64() + '0'));
    if (tmp <= 0) break;
  }
  std::reverse(out.begin(), out.end());
  return out;
}

ArithUint256 ArithUint256::fromString(const std::string& num) {
  ArithUint256 tmp = 0;
  for (char sym : num) {
    tmp *= 10;
    tmp += (sym - '0');
  }
  return tmp;
}

int ArithUint256::compareTo(const ArithUint256& b) const {
  for (int i = SHA256_HASH_SIZE - 1; i >= 0; i--) {
    if (data_[i] < b.data_[i]) {
      return -1;
    }
    if (data_[i] > b.data_[i]) {
      return 1;
    }
  }
  return 0;
}

ArithUint256& ArithUint256::operator<<=(unsigned int shift) {
  ArithUint256 a(*this);
  for (int i = 0; i < SHA256_HASH_SIZE; i++) {
    data_[i] = 0;
  }
  int k = shift / 8;
  shift = shift % 8;
  for (int i = 0; i < SHA256_HASH_SIZE; i++) {
    if (i + k + 1 < SHA256_HASH_SIZE && shift != 0) {
      data_[i + k + 1] |= (a.data_[i] >> (8 - shift));
    }
    if (i + k < SHA256_HASH_SIZE) {
      data_[i + k] |= static_cast<uint8_t>(a.data_[i] << shift);
    }
  }
  return *this;
}

ArithUint256& ArithUint256::operator>>=(unsigned int shift) {
  ArithUint256 a(*this);
  for (int i = 0; i < SHA256_HASH_SIZE; i++) {
    data_[i] = 0;
  }
  int k = shift / 8;
  shift = shift % 8;
  for (int i = 0; i < SHA256_HASH_SIZE; i++) {
    if (i - k - 1 >= 0 && shift != 0) {
      data_[i - k - 1] |= static_cast<uint8_t>(a.data_[i] << (8 - shift));
    }
    if (i - k >= 0) {
      data_[i - k] |= (a.data_[i] >> shift);
    }
  }
  return *this;
}

ArithUint256& ArithUint256::operator*=(uint32_t b32) {
  uint64_t carry = 0;
  for (int i = 0; i < SHA256_HASH_SIZE; i++) {
    uint64_t n = carry + (uint64_t)b32 * data_[i];
    data_[i] = n & 0xff;
    carry = n >> 8;
  }
  return *this;
}

ArithUint256& ArithUint256::operator*=(const ArithUint256& b) {
  ArithUint256 a;
  for (int j = 0; j < SHA256_HASH_SIZE; j++) {
    uint64_t carry = 0;
    for (int i = 0; i + j < SHA256_HASH_SIZE; i++) {
      uint64_t n = carry + a.data_[i + j] + (uint64_t)data_[j] * b.data_[i];
      a.data_[i + j] = n & 0xff;
      carry = n >> 8;
    }
  }
  *this = a;
  return *this;
}

ArithUint256& ArithUint256::operator/=(const ArithUint256& b) {
  ArithUint256 div = b;      // make a copy, so we can shift.
  ArithUint256 num = *this;  // make a copy, so we can subtract.
  *this = 0;                 // the quotient.
  int num_bits = num.bits();
  int div_bits = div.bits();
  if (div_bits == 0) {
    throw uint_error("Division by zero");
  }
  // the result is certainly 0.
  if (div_bits > num_bits) {
    return *this;
  }
  int shift = num_bits - div_bits;
  div <<= shift;  // shift so that div and num align.
  while (shift >= 0) {
    if (num >= div) {
      num -= div;
      data_[shift / 8] |= (1 << (shift & 7));  // set a bit of the result.
    }
    div >>= 1;  // shift back.
    shift--;
  }
  // num now contains the remainder of the division.
  return *this;
}

unsigned int ArithUint256::bits() const {
  for (int pos = SHA256_HASH_SIZE - 1; pos >= 0; pos--) {
    if (data_[pos] != 0) {
      for (int nbits = 7; nbits > 0; nbits--) {
        if ((data_[pos] & 1U << nbits) != 0) {
          return 8 * pos + nbits + 1;
        }
      }
      return 8 * pos + 1;
    }
  }
  return 0;
}

uint32_t ArithUint256::toBits(bool negative) const {
  int nSize = (bits() + 7) / 8;
  uint32_t nCompact = 0;
  if (nSize <= 3) {
    nCompact = (uint32_t)getLow64() << 8 * (3 - nSize);
  } else {
    ArithUint256 bn = *this >> 8 * (nSize - 3);
    nCompact = (uint32_t)bn.getLow64();
  }
  // The 0x00800000 bit denotes the sign.
  // Thus, if it is already set, divide the mantissa by 256 and increase the
  // exponent.
  if ((nCompact & 0x00800000) != 0) {
    nCompact >>= 8;
    nSize++;
  }
  VBK_ASSERT((nCompact & ~0x007fffff) == 0);
  VBK_ASSERT(nSize < 256);
  nCompact |= nSize << 24;
  nCompact |= (negative && (bool)(nCompact & 0x007fffff) ? 0x00800000 : 0);
  return nCompact;
}

uint64_t ArithUint256::getLow64() const {
  const uint32_t* p1 = (uint32_t*)(&data_[0]);
  const uint32_t* p2 = (uint32_t*)(&data_[4]);
  return *p1 | (uint64_t)*p2 << 32;
}

ArithUint256 ArithUint256::fromHex(const std::string& hex) {
  ArithUint256 u;
  u.setHex(hex);
  return u;
}

std::string ArithUint256::toHex() const {
  return HexStr(data_.rbegin(), data_.rend());
}

void ArithUint256::setHex(const std::string& value) {
  std::vector<uint8_t> bytes = ParseHex(value);
  if (bytes.size() > SHA256_HASH_SIZE) {
    throw uint_error("length of the number is more than 32 bytes");
  }

  std::copy(bytes.rbegin(), bytes.rend(), data_.begin());
}
