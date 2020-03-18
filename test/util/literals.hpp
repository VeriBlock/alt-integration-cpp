#ifndef ALT_INTEGRATION_LITERAPS_HPP
#define ALT_INTEGRATION_LITERAPS_HPP

#include <vector>

#include <veriblock/strutil.hpp>

inline std::vector<uint8_t> operator""_unhex(const char *str, size_t size) {
  std::string hex{str, str + size};
  return AltIntegrationLib::ParseHex(hex);
}

inline std::vector<uint8_t> operator""_v(const char *str, size_t size) {
  return std::vector<uint8_t>{str, str + size};
}

#endif  // ALT_INTEGRATION_LITERAPS_HPP
