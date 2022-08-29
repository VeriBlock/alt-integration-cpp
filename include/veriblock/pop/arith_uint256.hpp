// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_ARITH_UINT256_HPP
#define ALT_INTEGRATION_VERIBLOCK_ARITH_UINT256_HPP

#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "blob.hpp"
#include "consts.hpp"
#include "strutil.hpp"
#include "veriblock/pop/assert.hpp"

namespace altintegration {

//! @private
class uint_error : public std::runtime_error {
 public:
  explicit uint_error(const std::string& str) : std::runtime_error(str) {}
};

/**
 * @class ArithUint256
 *
 * 256-bit unsigned big integer.
 */
class ArithUint256 : public Blob<SHA256_HASH_SIZE> {
 public:
  ArithUint256() = default;

  ArithUint256(const std::vector<uint8_t>& v) : Blob<SHA256_HASH_SIZE>(v) {}

  // regular bytes should not be reversed
  template <size_t N>
  ArithUint256(const Blob<N>& b) {
    VBK_ASSERT(b.size() <= SHA256_HASH_SIZE);
    assign(b);
  }

  ArithUint256(uint64_t b) {
    data_[0] = (uint8_t)b;
    data_[1] = (uint8_t)(b >> 8);
    data_[2] = (uint8_t)(b >> 16);
    data_[3] = (uint8_t)(b >> 24);
    data_[4] = (uint8_t)(b >> 32);
    data_[5] = (uint8_t)(b >> 40);
    data_[6] = (uint8_t)(b >> 48);
    data_[7] = (uint8_t)(b >> 56);
    for (int i = 8; i < SHA256_HASH_SIZE; i++) {
      data_[i] = 0;
    }
  }

  const ArithUint256 operator~() const {
    ArithUint256 ret;
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
      ret.data_[i] = ~data_[i];
    }
    return ret;
  }

  const ArithUint256 operator-() const {
    ArithUint256 ret;
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
      ret.data_[i] = ~data_[i];
    }
    ++ret;
    return ret;
  }

  ArithUint256& operator=(uint64_t b) {
    data_[0] = (uint8_t)b;
    data_[1] = (uint8_t)(b >> 8);
    data_[2] = (uint8_t)(b >> 16);
    data_[3] = (uint8_t)(b >> 24);
    data_[4] = (uint8_t)(b >> 32);
    data_[5] = (uint8_t)(b >> 40);
    data_[6] = (uint8_t)(b >> 48);
    data_[7] = (uint8_t)(b >> 56);
    for (int i = 8; i < SHA256_HASH_SIZE; i++) {
      data_[i] = 0;
    }
    return *this;
  }

  ArithUint256& operator^=(const ArithUint256& b) {
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
      data_[i] ^= b.data_[i];
    }
    return *this;
  }

  ArithUint256& operator&=(const ArithUint256& b) {
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
      data_[i] &= b.data_[i];
    }
    return *this;
  }

  ArithUint256& operator|=(const ArithUint256& b) {
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
      data_[i] |= b.data_[i];
    }
    return *this;
  }

  ArithUint256& operator^=(uint64_t b) {
    data_[0] ^= (uint8_t)b;
    data_[1] ^= (uint8_t)(b >> 8);
    data_[2] ^= (uint8_t)(b >> 16);
    data_[3] ^= (uint8_t)(b >> 24);
    data_[4] ^= (uint8_t)(b >> 32);
    data_[5] ^= (uint8_t)(b >> 40);
    data_[6] ^= (uint8_t)(b >> 48);
    data_[7] ^= (uint8_t)(b >> 56);
    return *this;
  }

  ArithUint256& operator|=(uint64_t b) {
    data_[0] |= (uint8_t)b;
    data_[1] |= (uint8_t)(b >> 8);
    data_[2] |= (uint8_t)(b >> 16);
    data_[3] |= (uint8_t)(b >> 24);
    data_[4] |= (uint8_t)(b >> 32);
    data_[5] |= (uint8_t)(b >> 40);
    data_[6] |= (uint8_t)(b >> 48);
    data_[7] |= (uint8_t)(b >> 56);
    return *this;
  }

  ArithUint256& operator<<=(unsigned int shift);
  ArithUint256& operator>>=(unsigned int shift);

  ArithUint256& operator+=(const ArithUint256& b) {
    uint64_t carry = 0;
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
      uint64_t n = carry + data_[i] + b.data_[i];
      data_[i] = n & 0xff;
      carry = n >> 8;
    }
    return *this;
  }

  ArithUint256& operator-=(const ArithUint256& b) {
    *this += -b;
    return *this;
  }

  ArithUint256& operator+=(uint64_t b64) {
    ArithUint256 b;
    b = b64;
    *this += b;
    return *this;
  }

  ArithUint256& operator-=(uint64_t b64) {
    ArithUint256 b;
    b = b64;
    *this += -b;
    return *this;
  }

  ArithUint256& operator*=(uint32_t b32);
  ArithUint256& operator*=(const ArithUint256& b);
  ArithUint256& operator/=(const ArithUint256& b);

  ArithUint256& operator++() {
    // prefix operator
    for (int i = 0; i < SHA256_HASH_SIZE && ++data_[i] == 0; ++i) {
    }
    return *this;
  }

  const ArithUint256 operator++(int) {
    // postfix operator
    const ArithUint256 ret = *this;
    ++(*this);
    return ret;
  }

  ArithUint256& operator--() {
    // prefix operator
    for (int i = 0; i < SHA256_HASH_SIZE &&
                    --data_[i] == (std::numeric_limits<uint8_t>::max)();
         ++i) {
    }
    return *this;
  }

  const ArithUint256 operator--(int) {
    // postfix operator
    const ArithUint256 ret = *this;
    --(*this);
    return ret;
  }

  /**
   * Returns the position of the highest bit set plus one, or zero if the
   * value is zero.
   */
  unsigned int bits() const;

  int compareTo(const ArithUint256& b) const;

  std::string toString() const;

  static ArithUint256 fromString(const std::string& num);

  friend inline const ArithUint256 operator+(const ArithUint256& a,
                                             const ArithUint256& b) {
    return ArithUint256(a) += b;
  }
  friend inline const ArithUint256 operator-(const ArithUint256& a,
                                             const ArithUint256& b) {
    return ArithUint256(a) -= b;
  }
  friend inline const ArithUint256 operator*(const ArithUint256& a,
                                             const ArithUint256& b) {
    return ArithUint256(a) *= b;
  }
  friend inline const ArithUint256 operator/(const ArithUint256& a,
                                             const ArithUint256& b) {
    return ArithUint256(a) /= b;
  }
  friend inline const ArithUint256 operator|(const ArithUint256& a,
                                             const ArithUint256& b) {
    return ArithUint256(a) |= b;
  }
  friend inline const ArithUint256 operator&(const ArithUint256& a,
                                             const ArithUint256& b) {
    return ArithUint256(a) &= b;
  }
  friend inline const ArithUint256 operator^(const ArithUint256& a,
                                             const ArithUint256& b) {
    return ArithUint256(a) ^= b;
  }
  friend inline const ArithUint256 operator>>(const ArithUint256& a,
                                              int shift) {
    return ArithUint256(a) >>= shift;
  }
  friend inline const ArithUint256 operator<<(const ArithUint256& a,
                                              int shift) {
    return ArithUint256(a) <<= shift;
  }
  friend inline const ArithUint256 operator*(const ArithUint256& a,
                                             uint32_t b) {
    return ArithUint256(a) *= b;
  }
  friend inline bool operator>(const ArithUint256& a, const ArithUint256& b) {
    return a.compareTo(b) > 0;
  }
  friend inline bool operator<(const ArithUint256& a, const ArithUint256& b) {
    return a.compareTo(b) < 0;
  }
  friend inline bool operator>=(const ArithUint256& a, const ArithUint256& b) {
    return a.compareTo(b) >= 0;
  }
  friend inline bool operator<=(const ArithUint256& a, const ArithUint256& b) {
    return a.compareTo(b) <= 0;
  }
  friend inline bool operator==(const ArithUint256& a, uint64_t b) {
    return a.compareTo(b) == 0;
  }

  //! @throws uint_error
  static ArithUint256 fromHex(const std::string& hex);

  template <size_t N>
  static ArithUint256 fromLEBytes(const Blob<N>& b) {
    return ArithUint256(b.reverse());
  }

  // toHex should be inverted
  std::string toHex() const;

  uint64_t getLow64() const;

  static ArithUint256 fromBits(uint32_t bits,
                               bool* negative = nullptr,
                               bool* overflow = nullptr);

  uint32_t toBits(bool negative = false) const;

  void setHex(const std::string& value);
};

/// custom gtest printer, which prints Blob of any size as hexstring
//!@ private
inline void PrintTo(const ArithUint256& uint, ::std::ostream* os) {
  *os << uint.toHex();
}

}  // namespace altintegration

#endif  // BITCOIN_ARITH_UINT256_H
