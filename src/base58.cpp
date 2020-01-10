// Copyright (c) 2014-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/base58.hpp"

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace VeriBlock {

inline bool ValidAsCString(const std::string &str) noexcept {
  return str.size() == strlen(str.c_str());
}

/**
 * Tests if the given character is a whitespace character. The whitespace
 * characters are: space, form-feed ('\f'), newline ('\n'), carriage return
 * ('\r'), horizontal tab ('\t'), and vertical tab ('\v').
 *
 * This function is locale independent. Under the C locale this function gives
 * the same result as std::isspace.
 *
 * @param[in] c     character to test
 * @return          true if the argument is a whitespace character; otherwise
 * false
 */
constexpr inline bool IsSpace(char c) noexcept {
  return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' ||
         c == '\v';
}

/** All alphanumeric characters except for "0", "I", "O", and "l" */
static const char *pszBase58 =
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
static const int8_t mapBase58[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,  7,
    8,  -1, -1, -1, -1, -1, -1, -1, 9,  10, 11, 12, 13, 14, 15, 16, -1, 17, 18,
    19, 20, 21, -1, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1,
    -1, -1, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

static bool DecodeBase58(const char *psz,
                         std::vector<unsigned char> &vch,
                         size_t max_ret_len) {
  // Skip leading spaces.
  while (*psz && IsSpace(*psz)) psz++;
  // Skip and count leading '1's.
  size_t zeroes = 0;
  size_t length = 0;
  while (*psz == '1') {
    zeroes++;
    if (zeroes > max_ret_len) return false;
    psz++;
  }
  // Allocate enough space in big-endian base256 representation.
  size_t size =
      strlen(psz) * 733 / 1000 + 1;  // log(58) / log(256), rounded up.
  std::vector<unsigned char> b256(size);
  // Process the characters.
  static_assert(
      sizeof(mapBase58) / sizeof(mapBase58[0]) == 256,
      "mapBase58.size() should be 256");  // guarantee not out of range
  while (*psz && !IsSpace(*psz)) {
    // Decode base58 character
    int carry = mapBase58[(uint8_t)*psz];
    if (carry == -1)  // Invalid b58 character
      return false;
    size_t i = 0;
    for (auto it = b256.rbegin();
         (carry != 0 || i < length) && (it != b256.rend());
         ++it, ++i) {
      carry += 58 * (*it);
      *it = carry % 256;
      carry /= 256;
    }
    assert(carry == 0);
    length = i;
    if (length + zeroes > max_ret_len) return false;
    psz++;
  }
  // Skip trailing spaces.
  while (IsSpace(*psz)) psz++;
  if (*psz != 0) return false;
  // Skip leading zeroes in b256.
  auto it = b256.begin() + (size - length);
  // Copy result into output vector.
  vch.reserve(zeroes + (b256.end() - it));
  vch.assign(zeroes, 0x00);
  while (it != b256.end()) vch.push_back(*(it++));
  return true;
}

std::string EncodeBase58(const unsigned char *pbegin,
                         const unsigned char *pend) {
  // Skip & count leading zeroes.
  int zeroes = 0;
  int length = 0;
  while (pbegin != pend && *pbegin == 0) {
    pbegin++;
    zeroes++;
  }
  // Allocate enough space in big-endian base58 representation.
  size_t size =
      (pend - pbegin) * 138 / 100 + 1;  // log(256) / log(58), rounded up.
  std::vector<unsigned char> b58(size);
  // Process the bytes.
  while (pbegin != pend) {
    int carry = *pbegin;
    int i = 0;
    // Apply "b58 = b58 * 256 + ch".
    for (auto it = b58.rbegin();
         (carry != 0 || i < length) && (it != b58.rend());
         it++, i++) {
      carry += 256 * (*it);
      *it = carry % 58;
      carry /= 58;
    }

    assert(carry == 0);
    length = i;
    pbegin++;
  }
  // Skip leading zeroes in base58 result.
  auto it = b58.begin() + (size - length);
  while (it != b58.end() && *it == 0) it++;
  // Translate the result into a string.
  std::string str;
  str.reserve(zeroes + (b58.end() - it));
  str.assign(zeroes, '1');
  while (it != b58.end()) str += pszBase58[*(it++)];
  return str;
}

static bool DecodeBase58(const std::string &str,
                         std::vector<unsigned char> &vchRet,
                         size_t max_ret_len) {
  if (!ValidAsCString(str)) {
    return false;
  }
  return DecodeBase58(str.c_str(), vchRet, max_ret_len);
}

//std::string EncodeBase58(const uint8_t *buf, size_t size) {
//  return EncodeBase58(buf, buf + size);
//}

std::vector<uint8_t> DecodeBase58(const std::string &str) {
  std::vector<uint8_t> vch;
  bool status = DecodeBase58(str, vch, str.size() + 1);
  if (!status) {
    throw std::invalid_argument("DecodeBase58(): invalid input");
  };
  return vch;
}

}  // namespace Veriblock
