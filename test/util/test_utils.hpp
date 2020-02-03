#ifndef ALT_INTEGRATION_TEST_UTILS_HPP
#define ALT_INTEGRATION_TEST_UTILS_HPP

#include <stdint.h>

#include <algorithm>
#include <random>
#include <vector>

namespace VeriBlock {

template <typename T>
void generateRandomBytes(T begin, T end) {
  std::generate(begin, end, []() -> uint8_t { return (uint8_t)rand() % 256; });
}

inline std::vector<uint8_t> generateRandonBytesVector(size_t n) {
  std::vector<uint8_t> bytes(n);
  generateRandomBytes(bytes.begin(), bytes.end());
  return bytes;
}

}  // namespace VeriBlock

#endif
