// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/crypto/progpow/kiss99.hpp>

namespace altintegration {

uint32_t kiss99(kiss99_t &st) {
  st.z = 36969 * (st.z & 65535) + (st.z >> 16);
  st.w = 18000 * (st.w & 65535) + (st.w >> 16);
  uint32_t MWC = ((st.z << 16) + st.w);
  st.jsr ^= (st.jsr << 17);
  st.jsr ^= (st.jsr >> 13);
  st.jsr ^= (st.jsr << 5);
  st.jcong = 69069 * st.jcong + 1234567;
  return ((MWC ^ st.jcong) + st.jsr);
}

}  // namespace altintegration
