#include "veriblock/strutil.hpp"

#include <stdexcept>

namespace VeriBlock {

static const signed char p_util_hexdigit[256] = {
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0,  1,   2,   3,   4,   5,   6,   7,  8,  9,  -1, -1, -1, -1, -1, -1,
    -1, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

constexpr inline bool IsSpace(char c) noexcept {
  return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' ||
         c == '\v';
}

inline signed char HexDigit(char c) {
  return p_util_hexdigit[(unsigned char)c];
}

std::vector<uint8_t> ParseHex(const char *psz) {
  // convert hex dump to vector
  std::vector<uint8_t> vch;
  while (true) {
    while (IsSpace(*psz)) psz++;
    signed char c = HexDigit(*psz++);
    if (c == (int8_t)-1) break;
    uint8_t n = (c << 4u);
    c = HexDigit(*psz++);
    if (c == (int8_t)-1) break;
    n |= c;
    vch.push_back(n);
  }
  return vch;
}

std::vector<uint8_t> ParseHex(const std::string &hex) {
  return ParseHex(hex.c_str());
}

bool IsHex(const std::string &str) {
  for (char it : str) {
    if (HexDigit(it) < 0) return false;
  }
  return (!str.empty()) && (str.size() % 2 == 0);
}

}  // namespace VeriBlock