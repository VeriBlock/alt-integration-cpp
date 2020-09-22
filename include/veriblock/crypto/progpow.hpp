// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_PROGPOW_HPP
#define VERIBLOCK_POP_CPP_PROGPOW_HPP

#include <veriblock/serde.hpp>
#include <veriblock/uint.hpp>

namespace altintegration {

struct kiss99_t;

namespace progpow {

struct hash32_t {
  uint32_t uint32s[32 / sizeof(uint32_t)];

  hash32_t() {
    for (int i = 0; i < 8; i++) {
      uint32s[i] = 0;
    }
  }

  std::string toHex() const {
    char* u = (char*)&uint32s[0];
    return HexStr(u, u + 32);
  }

  bool operator==(const hash32_t& h) const {
    for (int i = 0; i < 8; i++) {
      if (uint32s[i] != h.uint32s[i]) {
        return false;
      }
    }

    return true;
  }

  static hash32_t readLE(ReadStream& rs) {
    hash32_t ret;
    for (int i = 0; i < 8; i++) {
      ret.uint32s[i] = rs.readLE<uint32_t>();
    }
    return ret;
  }

  static hash32_t readBE(ReadStream& rs) {
    hash32_t ret;
    for (int i = 0; i < 8; i++) {
      ret.uint32s[i] = rs.readBE<uint32_t>();
    }
    return ret;
  }
};

uint256 getVbkHeaderHash(Slice<const uint8_t> header);
void fill_mix(uint64_t seed, uint32_t lane_id, Slice<uint32_t> mix);
void keccak_f800(Slice<uint32_t> state);
hash32_t keccak_f800_progpow(const uint256& header,
                             uint64_t seed,
                             const hash32_t& digest);
hash32_t keccak_f800_progpow(const hash32_t& header,
                             uint64_t seed,
                             const hash32_t& digest);
uint32_t math(uint32_t a, uint32_t b, uint32_t r);
uint32_t merge(uint32_t a, uint32_t b, uint32_t r);
kiss99_t progPowInit(uint64_t prog_seed,
                     Slice<int> mix_seq_dst,
                     Slice<int> mix_seq_src);
}  // namespace progpow

uint192 progPowHash(Slice<const uint8_t> header);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_PROGPOW_HPP
