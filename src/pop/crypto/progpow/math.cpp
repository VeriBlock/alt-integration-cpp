// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <climits>
#include <veriblock/pop/crypto/progpow/math.hpp>

namespace altintegration {
namespace progpow {

uint32_t popcount(uint32_t v) {
#ifdef VBK_HAS_BUILTIN_POPCOUNT
  return __builtin_popcount(v);
#else
  using T = decltype(v);
  v = v - ((v >> 1) & (T) ~(T)0 / 3);                               // temp
  v = (v & (T) ~(T)0 / 15 * 3) + ((v >> 2) & (T) ~(T)0 / 15 * 3);   // temp
  v = (v + (v >> 4)) & (T) ~(T)0 / 255 * 15;                        // temp
  return (T)(v * ((T) ~(T)0 / 255)) >> (sizeof(T) - 1) * CHAR_BIT;  // count
#endif
}

uint32_t clz(uint32_t i) {
  if (i == 0) {
    return 32;
  }

#ifdef VBK_HAS_BUILTIN_CLZ
  return __builtin_clz(i);
#else
  int n = 31;
  if (i >= 65536) {
    n -= 16;
    i >>= 16;
  }

  if (i >= 256) {
    n -= 8;
    i >>= 8;
  }

  if (i >= 16) {
    n -= 4;
    i >>= 4;
  }

  if (i >= 4) {
    n -= 2;
    i >>= 2;
  }

  return n - (i >> 1);
#endif
}

uint32_t mul_hi(uint32_t a, uint32_t b) {
  return (uint64_t(a) * uint64_t(b)) >> 32;
}

}  // namespace progpow
}  // namespace altintegration