// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/base59.hpp>
#include <cstddef>
#include <array>
#include <limits>
#include <string>
#include <vector>
#include <cstdint>

#include "veriblock/pop/validation_state.hpp"

namespace altintegration {

static const char *g_Base59Alphabet =
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz0";
static const uint32_t g_kBase_256 = 256;
static const size_t g_kBase59 = 59;  // strlen(g_Base59Alphabet)

static const std::array<int8_t, 128> g_Indexes = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 58, 0,  1,  2,  3,  4,  5,  6,  7,
    8,  -1, -1, -1, -1, -1, -1, -1, 9,  10, 11, 12, 13, 14, 15, 16, -1, 17, 18,
    19, 20, 21, -1, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1,
    -1, -1, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, -1, -1, -1, -1, -1};

static uint8_t divmod59(std::vector<uint8_t> &number, size_t startAt) {
  size_t remainder = 0;
  for (size_t i = startAt; i < number.size(); i++) {
    uint32_t digit256 = (uint32_t)number[i] & 0xFF;
    size_t temp = remainder * g_kBase_256 + digit256;
    number[i] = (uint8_t)(temp / g_kBase59);
    remainder = temp % g_kBase59;
  }
  return (uint8_t)remainder;
}

static uint8_t divmod256(std::vector<uint8_t> &number59, size_t startAt) {
  size_t remainder = 0;
  for (size_t i = startAt; i < number59.size(); i++) {
    int digit59 = (int8_t)number59[i] & 0xFF;
    size_t temp = remainder * g_kBase59 + digit59;
    number59[i] = (uint8_t)(temp / g_kBase_256);
    remainder = temp % g_kBase_256;
  }
  return (uint8_t)remainder;
}

std::string EncodeBase59(const uint8_t *buf, size_t nSize) {
  if (nSize == 0u) {
    return {};
  }

  // Make a copy of the input since we are going to modify it.
  std::vector<uint8_t> input(buf, buf + nSize);

  // Count leading zeroes
  size_t zeroCount = 0;
  while (zeroCount < nSize && input[zeroCount] == 0) {
    ++zeroCount;
  }

  // The actual encoding
  std::vector<uint8_t> temp(nSize * 2);
  size_t j = temp.size();

  size_t startAt = zeroCount;
  while (startAt < input.size()) {
    uint8_t mod = divmod59(input, startAt);
    if (input[startAt] == 0) {
      ++startAt;
    }
    temp[--j] = g_Base59Alphabet[mod];
  };

  // Strip extra '1' if any
  while (j < temp.size() && temp[j] == g_Base59Alphabet[0]) {
    ++j;
  }

  // Add as many leading '1' as there were leading zeros.
  for (--zeroCount; zeroCount != std::numeric_limits<size_t>::max();
       zeroCount--) {
    temp[--j] = g_Base59Alphabet[0];
  }

  return std::string{temp.begin() + j, temp.end()};
};

std::string EncodeBase59(const unsigned char *pbegin,
                         const unsigned char *pend) {
  return EncodeBase59(pbegin, pend - pbegin);
};

bool DecodeBase59(const std::string &input,
                  std::vector<uint8_t> &out,
                  ValidationState &state) {
  if (input.empty()) {
    // empty input is a valid base59
    return true;
  }

  std::vector<uint8_t> input59(input.size());

  // Transform the String to a base59 byte sequence
  for (size_t i = 0; i < input.size(); ++i) {
    int8_t digit59 = g_Indexes[input[i]];
    if (digit59 < 0) {
      return state.Invalid("decode-base-59", "invalid b59 character");
    }

    input59[i] = digit59;
  }

  // Count leading zeroes
  size_t zeroCount = 0;
  while (zeroCount < input59.size() && input59[zeroCount] == 0) {
    ++zeroCount;
  }

  // The encoding
  std::vector<uint8_t> temp(input.size());
  size_t j = temp.size();

  size_t startAt = zeroCount;
  while (startAt < input59.size()) {
    uint8_t mod = divmod256(input59, startAt);
    if (input59[startAt] == 0) {
      ++startAt;
    }

    temp[--j] = mod;
  }

  // Do no add extra leading zeroes, move j to first non null byte.
  while (j < temp.size() && temp[j] == 0) {
    ++j;
  }

  out = std::vector<uint8_t>{temp.begin() + j - zeroCount, temp.end()};
  return true;
}

}  // namespace altintegration
