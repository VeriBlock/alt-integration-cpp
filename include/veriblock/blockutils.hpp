#include "entities/hashes.hpp"

namespace VeriBlock {

inline uint256 decodeBits(const uint32_t& bits) {
  std::vector<uint8_t> target(SHA256_HASH_SIZE);
  int nSize = bits >> 24;
  uint32_t nWord = bits & 0x007fffff;
  if (nSize <= 3) {
    nWord >>= 8 * (3 - nSize);
    target[0] = (uint8_t)nWord;
    target[1] = (uint8_t)(nWord >> 8);
    target[2] = (uint8_t)(nWord >> 16);
    target[3] = (uint8_t)(nWord >> 24);

    return uint256(std::vector<uint8_t>(target.rbegin(), target.rend()));
  } else {
    target[0] = (uint8_t)nWord;
    target[1] = (uint8_t)(nWord >> 8);
    target[2] = (uint8_t)(nWord >> 16);
    target[3] = (uint8_t)(nWord >> 24);
    std::vector<uint8_t> temp(SHA256_HASH_SIZE);
    for (int i = (nSize - 3), j = 0; i < SHA256_HASH_SIZE; ++i, ++j) {
      temp[i] = target[j];
    }

    return uint256(std::vector<uint8_t>(temp.rbegin(), temp.rend()));
  }
}
// TODO: write the encodeBits function
inline uint32_t encodeBits(const uint256& target);
}  // namespace VeriBlock
