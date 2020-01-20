#include "veriblock/shautil.hpp"

namespace VeriBlock {

Sha256Hash sha256get(Slice<uint8_t> data) {
  uint8_t btctxHash[SHA256_HASH_SIZE]{};
  sha256(btctxHash, data.data(), (uint32_t)data.size());
  Slice<uint8_t> result(btctxHash, SHA256_HASH_SIZE);
  return Sha256Hash(result);
}

Sha256Hash sha256twice(Slice<uint8_t> data) {
  auto firstShot = sha256get(data);
  return sha256get(firstShot);
}

}  // namespace VeriBlock
