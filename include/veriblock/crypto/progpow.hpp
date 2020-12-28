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

//! @private
namespace progpow {

struct hash32_t {
  uint32_t uint32s[32 / sizeof(uint32_t)];

  hash32_t();
  std::string toHex() const;
  bool operator==(const hash32_t& h) const;
  static hash32_t readLE(ReadStream& rs);
  static hash32_t readBE(ReadStream& rs);
};

uint256 getVbkHeaderHash(Slice<const uint8_t> header);
void fill_mix(uint64_t seed, uint32_t lane_id, Slice<uint32_t> mix);
void keccak_f800(Slice<uint32_t> state);
void keccak_f800_2(Slice<uint32_t> state);
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

struct ethash_cache;

inline uint32_t ethashGetEpochWithoutOffset(int64_t height) {
  return (uint32_t)(height / VBK_ETHASH_EPOCH_LENGTH);
}

inline uint32_t ethashGetEpoch(int64_t height) {
  return ethashGetEpochWithoutOffset(height) + VBK_ETHASH_EPOCH_OFFSET;
}

void insertHeaderCacheEntry(Slice<const uint8_t> header, uint192 progpowHash);
void clearHeaderCache();
void clearEthashCache();

}  // namespace progpow

void clearEthashCache();

/**
 * Calculate vPROGPOW hash of given VbkBlock header (65 bytes)
 *
 * @param header 65-bytes header
 * @return 24-byte hash
 */
uint192 progPowHash(Slice<const uint8_t> header);

//! @overload
uint192 progPowHash(Slice<const uint8_t> header, progpow::ethash_cache* light);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_PROGPOW_HPP
