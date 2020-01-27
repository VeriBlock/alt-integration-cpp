#include "veriblock/arith_uint256.hpp"

using namespace VeriBlock;

ArithUint256& ArithUint256::decodeBits(const uint32_t& bits) {
  ArithUint256 target;
  int nSize = bits >> 24;
  uint32_t nWord = bits & 0x007fffff;
  if (nSize <= 3) {
    nWord >>= 8 * (3 - nSize);
    data_[0] = (uint8_t)nWord;
    data_[1] = (uint8_t)(nWord >> 8);
    data_[2] = (uint8_t)(nWord >> 16);
    data_[3] = (uint8_t)(nWord >> 24);
  } else {
    data_[0] = (uint8_t)nWord;
    data_[1] = (uint8_t)(nWord >> 8);
    data_[2] = (uint8_t)(nWord >> 16);
    data_[3] = (uint8_t)(nWord >> 24);
    *this <<= 8 * (nSize - 3);
  }
  return *this;
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
      data_[i + k] |= (a.data_[i] << shift);
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
      data_[i - k - 1] |= (a.data_[i] << (8 - shift));
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
    if (data_[pos]) {
      for (int nbits = 7; nbits > 0; nbits--) {
        if (data_[pos] & 1U << nbits) {
          return 8 * pos + nbits + 1;
        }
      }
      return 8 * pos + 1;
    }
  }
  return 0;
}
