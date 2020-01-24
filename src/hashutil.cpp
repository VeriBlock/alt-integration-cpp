#include <cassert>

#include "veriblock/hashutil.hpp"

namespace VeriBlock {

uint256 sha256(Slice<const uint8_t> data) {
  assert(data.size() <= std::numeric_limits<uint32_t>::max());
  uint256 btctxHash{};
  sha256(btctxHash.data(), data.data(), (uint32_t)data.size());
  return btctxHash;
}

uint256 sha256twice(Slice<const uint8_t> data) {
  auto firstShot = sha256(data);
  return sha256(firstShot);
}

uint192 vblake(Slice<const uint8_t> data) {
  assert(data.size() <= std::numeric_limits<uint32_t>::max());
  uint192 hash{};
  vblake(hash.data(), data.data(), (uint32_t)data.size());
  return hash;
}

}  // namespace VeriBlock
