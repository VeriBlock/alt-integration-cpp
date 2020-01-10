#ifndef ALT_INTEGRATION_LITERAPS_HPP
#define ALT_INTEGRATION_LITERAPS_HPP

#include "veriblock/strutil.hpp"
#include <vector>

inline std::vector<uint8_t> operator""_unhex(const char *str, size_t size) {
  std::string hex{str, str + size};
  return VeriBlock::ParseHex(hex);
}

#endif // ALT_INTEGRATION_LITERAPS_HPP
