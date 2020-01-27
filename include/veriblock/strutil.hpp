#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STRUTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STRUTIL_HPP_

#include <string>
#include <vector>

#include "base58.hpp"
#include "base59.hpp"

namespace VeriBlock {

bool IsHex(const std::string& str);
std::vector<uint8_t> ParseHex(const char* psz);
std::vector<uint8_t> ParseHex(const std::string& hex);

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

template <typename T>
inline std::string HexStr(const T& vch) {
  return HexStr(vch.begin(), vch.end());
}

std::vector<uint8_t> toBytes(const std::string input);

}  // namespace VeriBlock
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STRUTIL_HPP_
