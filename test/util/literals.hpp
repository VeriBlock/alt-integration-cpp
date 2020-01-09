#ifndef ALT_INTEGRATION_LITERAPS_HPP
#define ALT_INTEGRATION_LITERAPS_HPP

#include "util.hpp"
#include <vector>

std::vector<uint8_t> operator""_unhex(const char *str, size_t size) {
  return HexToBytes({str, str + size});
}

#endif // ALT_INTEGRATION_LITERAPS_HPP
