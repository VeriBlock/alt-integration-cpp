#include "veriblock/uint.hpp"

namespace VeriBlock {

uint256 decodeBits(const uint32_t& bits) {
  uint256 target{};
  uint32_t nSize = bits >> 24u;
  uint32_t nWord = bits & 0x007fffffu;
  if (nSize <= 3u) {
    nWord >>= 8u * (3u - nSize);
    target[0] = (uint8_t)nWord;
    target[1] = (uint8_t)(nWord >> 8u);
    target[2] = (uint8_t)(nWord >> 16u);
    target[3] = (uint8_t)(nWord >> 24u);

    return target.reverse();
  } else {
    target[0] = (uint8_t)nWord;
    target[1] = (uint8_t)(nWord >> 8u);
    target[2] = (uint8_t)(nWord >> 16u);
    target[3] = (uint8_t)(nWord >> 24u);
    uint256 temp{};
    for (uint32_t i = (nSize - 3), j = 0; i < SHA256_HASH_SIZE; ++i, ++j) {
      temp[i] = target[j];
    }

    return temp.reverse();
  }
}
}  // namespace VeriBlock