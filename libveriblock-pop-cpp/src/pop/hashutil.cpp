// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/hashutil.hpp>

#include <veriblock/pop/assert.hpp>

namespace altintegration {

uint256 sha256(Slice<const uint8_t> data) {
  VBK_ASSERT(data.size() <= (std::numeric_limits<uint32_t>::max)());
  uint256 btctxHash{};
  sha256(btctxHash.data(), data.data(), (uint32_t)data.size());
  return btctxHash;
}

uint256 sha256twice(Slice<const uint8_t> data) {
  auto firstShot = sha256(data);
  return sha256(firstShot);
}

uint256 sha256twice(Slice<const uint8_t> a, Slice<const uint8_t> b) {
  auto firstShot = sha256(a, b);
  return sha256(firstShot);
}

uint256 sha256(Slice<const uint8_t> a, Slice<const uint8_t> b) {
  sha256_context ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, a.data(), (uint32_t)a.size());
  sha256_update(&ctx, b.data(), (uint32_t)b.size());

  uint256 ret;
  sha256_finish(&ctx, ret.data());

  return ret;
}

uint192 vblake(Slice<const uint8_t> data) {
  VBK_ASSERT(data.size() <= 64);
  uint192 hash{};
  int ret = vblake(hash.data(), data.data(), (uint32_t)data.size());
  VBK_ASSERT(ret >= 0);
  return hash;
}

}  // namespace altintegration
