#include "veriblock/hashutil.hpp"

#include <cassert>

namespace VeriBlock {

Sha256Hash sha256(Slice<uint8_t> data) {
  assert(data.size() <= std::numeric_limits<uint32_t>::max());
  Sha256Hash btctxHash{};
  sha256(btctxHash.data(), data.data(), (uint32_t)data.size());
  return btctxHash;
}

Sha256Hash sha256twice(Slice<uint8_t> data) {
  auto firstShot = sha256(data);
  return sha256(firstShot);
}

}  // namespace VeriBlock
