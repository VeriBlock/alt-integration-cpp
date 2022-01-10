// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_STRUTIL_HPP
#define ALT_INTEGRATION_VERIBLOCK_STRUTIL_HPP

#include <string>
#include <vector>

#include "base58.hpp"
#include "base59.hpp"

namespace altintegration {

//! @private
bool IsHex(const std::string& str);

//! Parse bytes from hex.
std::vector<uint8_t> ParseHex(const char* psz);
//! @overload
std::vector<uint8_t> ParseHex(const std::string& hex);

//! @private
constexpr inline bool IsSpace(char c) noexcept {
  return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' ||
         c == '\v';
}

//! Convert bytes to hex.
template <typename T>
std::string HexStr(const T itbegin, const T itend) {
  std::string rv;
  // clang-format off
  static const char hexmap[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
  // clang-format on
  rv.reserve(std::distance(itbegin, itend) * 2u);
  for (T it = itbegin; it < itend; ++it) {
    auto val = (uint8_t)(*it);
    rv.push_back(hexmap[val >> 4u]);
    rv.push_back(hexmap[val & 15u]);
  }
  return rv;
}

//! @overload
template <typename T>
inline std::string HexStr(const T& vch, bool reverseHex = false) {
  if(reverseHex) {
    return HexStr(vch.rbegin(), vch.rend());
  } else {
    return HexStr(vch.begin(), vch.end());
  }
}

//! @private
std::vector<uint8_t> toBytes(const std::string& input);



}  // namespace altintegration
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STRUTIL_HPP_
