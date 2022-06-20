// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <cstdint>
#include <mutex>
#include <utility>
#include <vector>
#include <veriblock/pop/assert.hpp>
#include <veriblock/pop/cache/small_lfru_cache.hpp>
#include <veriblock/pop/consts.hpp>
#include <veriblock/pop/crypto/progpow.hpp>
#include <veriblock/pop/crypto/progpow/cache.hpp>
#include <veriblock/pop/crypto/progpow/ethash.hpp>
#include <veriblock/pop/crypto/progpow/kiss99.hpp>
#include <veriblock/pop/crypto/progpow/math.hpp>
#include <veriblock/pop/finalizer.hpp>
#include <veriblock/pop/hashutil.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/slice.hpp>
#include <veriblock/pop/storage/ethash_cache_provider.hpp>
#include <veriblock/pop/third_party/lru_cache.hpp>
#include <veriblock/pop/trace.hpp>

#include "libethash/internal.hpp"

#define PROGPOW_PERIOD 10
#define PROGPOW_LANES 16
#define PROGPOW_REGS 32
#define PROGPOW_DAG_LOADS 4
#define PROGPOW_CNT_DAG 128
#define PROGPOW_CNT_CACHE 11
#define PROGPOW_CNT_MATH 20
#define PROGPOW_CACHE_BYTES (16 * 1024)
#define PROGPOW_CACHE_WORDS (PROGPOW_CACHE_BYTES / sizeof(uint32_t))
#define FNV_OFFSET_BASIS 0x811c9dc5
#define FNV_PRIME 0x1000193
#define DATASET_PARENTS 256

namespace altintegration {
namespace progpow {

inline uint32_t rotl32(uint32_t value, unsigned int count) {
  const unsigned int mask = CHAR_BIT * sizeof(value) - 1;
  count &= mask;
  return (value << count) |
         (value >> ((std::numeric_limits<uint32_t>::max() + 1 - count) & mask));
}

inline uint32_t rotr32(uint32_t value, unsigned int count) {
  const unsigned int mask = CHAR_BIT * sizeof(value) - 1;
  count &= mask;
  return (value >> count) |
         (value << ((std::numeric_limits<uint32_t>::max() + 1 - count) & mask));
}

inline uint32_t fnv1a(uint32_t h, uint32_t d) { return (h ^ d) * FNV_PRIME; }

uint32_t merge(uint32_t a, uint32_t b, uint32_t r) {
  switch (r % 4) {
    case 0: {
      auto count = (((r >> 16) % 31) + 1);
      return rotr32(a, count) ^ b;
    }
    case 1: {
      auto count = (((r >> 16) % 31) + 1);
      return rotl32(a, count) ^ b;
    }
    case 2: {
      return a * 33 + b;
    }
    case 3: {
      return (a ^ b) * 33;
    }
    default:
      VBK_ASSERT(false);
  }
}

uint32_t math(uint32_t a, uint32_t b, uint32_t r) {
  // clang-format off
  switch(r % 11) {
    case 0: return rotl32(a, b);
    case 1: return a & b;
    case 2: return a + b;
    case 3: return popcount(a) + popcount(b);
    case 4: return clz(a) + clz(b);
    case 5: return rotr32(a, b);
    case 6: return mul_hi(a, b);
    case 7: return a | b;
    case 8: return a * b;
    case 9: return a ^ b;
    case 10: return std::min(a,b);
    default: VBK_ASSERT(false);
  }
  // clang-format on
}

static uint64_t getVbkBlockNonce(Slice<const uint8_t> header) {
  VBK_ASSERT(header.size() == VBK_HEADER_SIZE_PROGPOW);
  // last 5 bytes
  Slice<const uint8_t> nonceBytes(header.begin() + VBK_HEADER_SIZE_PROGPOW - 5,
                                  5);

  auto initialShift = 32;
  uint64_t nonce = 0;
  int bytePointer = 0;
  for (int shift = initialShift; shift >= 0; shift -= 8) {
    nonce += (0xFFLL & (nonceBytes[bytePointer++])) << shift;
  }
  return nonce;
}

static uint64_t getVbkBlockHeight(Slice<const uint8_t> header) {
  VBK_ASSERT(header.size() == VBK_HEADER_SIZE_PROGPOW);
  // first 4 bytes
  Slice<const uint8_t> heightBytes(header.begin(), 4);

  int blockHeight = 0;
  int bytePointer = 0;
  for (int shift = 24; shift >= 0; shift -= 8) {
    blockHeight += (0xFF & (heightBytes[bytePointer++])) << shift;
  }
  return blockHeight;
}

static const uint32_t keccakf_rndc[24] = {
    0x00000001, 0x00008082, 0x0000808a, 0x80008000, 0x0000808b, 0x80000001,
    0x80008081, 0x00008009, 0x0000008a, 0x00000088, 0x80008009, 0x8000000a,
    0x8000808b, 0x0000008b, 0x00008089, 0x00008003, 0x00008002, 0x00000080,
    0x0000800a, 0x8000000a, 0x80008081, 0x00008080, 0x80000001, 0x80008008};

inline int right3(int foo, int n) { return (int)((uint32_t)foo >> n); }

void keccak_f800(Slice<uint32_t> state) {
  int a00 = state[0], a01 = state[1], a02 = state[2], a03 = state[3],
      a04 = state[4];
  int a05 = state[5], a06 = state[6], a07 = state[7], a08 = state[8],
      a09 = state[9];
  int a10 = state[10], a11 = state[11], a12 = state[12], a13 = state[13],
      a14 = state[14];
  int a15 = state[15], a16 = state[16], a17 = state[17], a18 = state[18],
      a19 = state[19];
  int a20 = state[20], a21 = state[21], a22 = state[22], a23 = state[23],
      a24 = state[24];

  for (int i = 0; i < 22; i++) {
    // theta
    int c0 = a00 ^ a06 ^ a09 ^ a12 ^ a17;
    int c1 = a08 ^ a11 ^ a14 ^ a19 ^ a23;
    int c2 = a02 ^ a07 ^ a10 ^ a18 ^ a22;
    int c3 = a04 ^ a05 ^ a15 ^ a20 ^ a24;
    int c4 = a01 ^ a03 ^ a13 ^ a16 ^ a21;

    int d1 = (c1 << 1 | right3(c1, 31)) ^ c4;
    int d2 = (c2 << 1 | right3(c2, 31)) ^ c0;
    int d3 = (c3 << 1 | right3(c3, 31)) ^ c1;
    int d4 = (c4 << 1 | right3(c4, 31)) ^ c2;
    int d0 = (c0 << 1 | right3(c0, 31)) ^ c3;

    a00 ^= d1;
    a05 ^= d1;
    a10 ^= d1;
    a15 ^= d1;
    a20 ^= d1;
    a01 ^= d2;
    a06 ^= d2;
    a11 ^= d2;
    a16 ^= d2;
    a21 ^= d2;
    a02 ^= d3;
    a07 ^= d3;
    a12 ^= d3;
    a17 ^= d3;
    a22 ^= d3;
    a03 ^= d4;
    a08 ^= d4;
    a13 ^= d4;
    a18 ^= d4;
    a23 ^= d4;
    a04 ^= d0;
    a09 ^= d0;
    a14 ^= d0;
    a19 ^= d0;
    a24 ^= d0;

    // rho/pi
    c1 = a01 << 1 | right3(a01, 31);
    a01 = a06 << 12 | right3(a06, 20);
    a06 = a09 << 20 | right3(a09, 12);
    a09 = a22 << 29 | right3(a22, 3);
    a22 = a14 << 7 | right3(a14, 25);
    a14 = a20 << 18 | right3(a20, 14);
    a20 = a02 << 30 | right3(a02, 2);
    a02 = a12 << 11 | right3(a12, 21);
    a12 = a13 << 25 | right3(a13, 7);
    a13 = a19 << 8 | right3(a19, 24);
    a19 = a23 << 24 | right3(a23, 8);
    a23 = a15 << 9 | right3(a15, 23);
    a15 = a04 << 27 | right3(a04, 5);
    a04 = a24 << 14 | right3(a24, 18);
    a24 = a21 << 2 | right3(a21, 30);
    a21 = a08 << 23 | right3(a08, 9);
    a08 = a16 << 13 | right3(a16, 19);
    a16 = a05 << 4 | right3(a05, 28);
    a05 = a03 << 28 | right3(a03, 4);
    a03 = a18 << 21 | right3(a18, 11);
    a18 = a17 << 15 | right3(a17, 17);
    a17 = a11 << 10 | right3(a11, 22);
    a11 = a07 << 6 | right3(a07, 26);
    a07 = a10 << 3 | right3(a10, 29);
    a10 = c1;

    a03 ^= 0x79938B61;
    a10 ^= ((a19 & 0x000000FF) | (a24 & 0x0000FF00) | (a06 & 0x00FF0000) |
            (a14 & 0xFF000000));

    // chi
    c0 = a00 ^ (~a01 & a02);
    c1 = a01 ^ (~a02 & a03);
    a02 ^= ~a03 & a04;
    a03 ^= ~a04 & a00;
    a04 ^= ~a00 & a01;
    a00 = c0;
    a01 = c1;

    c0 = a05 ^ (~a06 & a07);
    c1 = a06 ^ (~a07 & a08);
    a07 ^= ~a08 & a09;
    a08 ^= ~a09 & a05;
    a09 ^= ~a05 & a06;
    a05 = c0;
    a06 = c1;

    c0 = a10 ^ (~a11 & a12);
    c1 = a11 ^ (~a12 & a13);
    a12 ^= ~a13 & a14;
    a13 ^= ~a14 & a10;
    a14 ^= ~a10 & a11;
    a10 = c0;
    a11 = c1;

    c0 = a15 ^ (~a16 & a17);
    c1 = a16 ^ (~a17 & a18);
    a17 ^= ~a18 & a19;
    a18 ^= ~a19 & a15;
    a19 ^= ~a15 & a16;
    a15 = c0;
    a16 = c1;

    c0 = a20 ^ (~a21 & a22);
    c1 = a21 ^ (~a22 & a23);
    a22 ^= ~a23 & a24;
    a23 ^= ~a24 & a20;
    a24 ^= ~a20 & a21;
    a20 = c0;
    a21 = c1;

    // iota
    a00 ^= keccakf_rndc[i];
  }

  state[0] = a00;
  state[1] = a01;
  state[2] = a02;
  state[3] = a03;
  state[4] = a04;
  state[5] = a05;
  state[6] = a06;
  state[7] = a07;
  state[8] = a08;
  state[9] = a09;
  state[10] = a10;
  state[11] = a11;
  state[12] = a12;
  state[13] = a13;
  state[14] = a14;
  state[15] = a15;
  state[16] = a16;
  state[17] = a17;
  state[18] = a18;
  state[19] = a19;
  state[20] = a20;
  state[21] = a21;
  state[22] = a22;
  state[23] = a23;
  state[24] = a24;
}

hash32_t keccak_f800_progpow(const uint256& header,
                             uint64_t seed,
                             const hash32_t& digest) {
  hash32_t hdr;
  ReadStream rh(header);
  for (int i = 0; i < 8; i++) {
    hdr.uint32s[i] = rh.assertReadLE<uint32_t>();
  }
  return keccak_f800_progpow(hdr, seed, digest);
}

hash32_t keccak_f800_progpow(const hash32_t& header,
                             uint64_t seed,
                             const hash32_t& digest) {
  const int st_size = 25;
  uint32_t st[st_size];
  // Initialization
  for (int i = 0; i < st_size; i++) {
    st[i] = 0;
  }

  // Absorb phase for fixed 18 words of input
  for (int i = 0; i < 8; i++) {
    st[i] = header.uint32s[i];
  }
  st[8] = uint32_t(seed);
  st[9] = uint32_t(seed >> 32);
  for (int i = 0; i < 8; i++) {
    st[10 + i] = digest.uint32s[i];
  }

  // keccak_f800 call for the single absorb pass
  keccak_f800({st, st_size});

  // Squeeze phase for fixed 8 words of output
  hash32_t ret;
  for (int i = 0; i < 8; i++) {
    ret.uint32s[i] = st[i];
  }

  return ret;
}

void fill_mix(uint64_t seed, uint32_t lane_id, Slice<uint32_t> mix) {
  // Use FNV to expand the per-warp seed to per-lane
  // Use KISS to expand the per-lane seed to fill mix
  kiss99_t st;
  st.z = fnv1a(FNV_OFFSET_BASIS, uint32_t(seed));
  st.w = fnv1a(st.z, uint32_t(seed >> 32));
  st.jsr = fnv1a(st.w, lane_id);
  st.jcong = fnv1a(st.jsr, lane_id);
  for (int i = 0; i < PROGPOW_REGS; i++) {
    mix[i] = kiss99(st);
  }
}

kiss99_t progPowInit(uint64_t prog_seed,
                     Slice<int> mix_seq_src,
                     Slice<int> mix_seq_dst) {
  uint32_t leftSeed = uint32_t(prog_seed >> 32);
  uint32_t rightSeed = uint32_t(prog_seed);

  kiss99_t prog_rnd;
  prog_rnd.z = fnv1a(FNV_OFFSET_BASIS, rightSeed);
  prog_rnd.w = fnv1a(prog_rnd.z, leftSeed);
  prog_rnd.jsr = fnv1a(prog_rnd.w, rightSeed);
  prog_rnd.jcong = fnv1a(prog_rnd.jsr, leftSeed);
  // Create a random sequence of mix destinations for merge() and mix sources
  // for cache reads guarantees every destination merged once guarantees no
  // duplicate cache reads, which could be optimized away Uses Fisher-Yates
  // shuffle
  for (int i = 0; i < PROGPOW_REGS; i++) {
    mix_seq_dst[i] = i;
    mix_seq_src[i] = i;
  }
  for (int i = PROGPOW_REGS - 1, j = 0; i > 0; i--) {
    j = kiss99(prog_rnd) % (i + 1);
    std::swap(mix_seq_dst[i], mix_seq_dst[j]);
    j = kiss99(prog_rnd) % (i + 1);
    std::swap(mix_seq_src[i], mix_seq_src[j]);
  }
  return prog_rnd;
}

void progPowLoop(const uint64_t block_number,
                 const uint64_t loop,
                 uint32_t mix[PROGPOW_LANES][PROGPOW_REGS],
                 const std::vector<uint32_t>& dag,
                 ethash_cache* light) {
  uint64_t dag_bytes = ethash_get_datasize(block_number);

  // dag_entry holds the 256 bytes of data loaded from the DAG
  uint32_t dag_entry[PROGPOW_LANES][PROGPOW_DAG_LOADS];
  // On each loop iteration rotate which lane is the source of the DAG address.
  // The source lane's mix[0] value is used to ensure the last loop's DAG data
  // feeds into this loop's address. dag_addr_base is which 256-byte entry
  // within the DAG will be accessed

  int dag_addr_base =
      mix[loop % PROGPOW_LANES][0] %
      (dag_bytes / (PROGPOW_LANES * PROGPOW_DAG_LOADS * sizeof(uint32_t)));

  uint32_t lastLookup = 0xffffffff;
  ethash_dag_node_t lookupNode;

  for (int l = 0; l < PROGPOW_LANES; l++) {
    // Lanes access DAG_LOADS sequential words from the dag entry
    // Shuffle which portion of the entry each lane accesses each iteration by
    // XORing lane and loop. This prevents multi-chip ASICs from each storing
    // just a portion of the DAG
    int offset = (uint32_t(l) ^ loop) % PROGPOW_LANES;
    int dag_addr_lane = dag_addr_base * PROGPOW_LANES + offset;
    uint32_t lookup = dag_addr_lane / 4 + (offset >> 4);

    if (lookup != lastLookup) {
      lastLookup = lookup;
      ethash_calculate_dag_node(&lookupNode, lookup, light);
    }
    for (int i = 0; i < PROGPOW_DAG_LOADS; i++) {
      uint32_t lookupOffset =
          ((i * 4 + ((offset & 0xf) << 4)) % 64) / sizeof(uint32_t);
      uint32_t j = lookupNode.words[lookupOffset];
      dag_entry[l][i] = j;
    }
  }

  // Initialize the program seed and sequences
  // When mining these are evaluated on the CPU and compiled away
  std::array<int, PROGPOW_REGS> mix_seq_dst;
  std::array<int, PROGPOW_REGS> mix_seq_src;
  int mix_seq_dst_cnt = 0;
  int mix_seq_src_cnt = 0;

  uint64_t prog_seed =
      (block_number + VBK_ETHASH_EPOCH_LENGTH * VBK_ETHASH_EPOCH_OFFSET) /
      PROGPOW_PERIOD;
  kiss99_t prog_rnd = progPowInit(prog_seed, mix_seq_src, mix_seq_dst);

  int max_i = std::max(PROGPOW_CNT_CACHE, PROGPOW_CNT_MATH);
  for (int i = 0; i < max_i; i++) {
    if (i < PROGPOW_CNT_CACHE) {
      // Cached memory access
      // lanes access random 32-bit locations within the first portion of the
      // DAG
      int src = mix_seq_src[(mix_seq_src_cnt++) % PROGPOW_REGS];
      int dst = mix_seq_dst[(mix_seq_dst_cnt++) % PROGPOW_REGS];
      int sel = kiss99(prog_rnd);
      for (int l = 0; l < PROGPOW_LANES; l++) {
        uint32_t offset =
            mix[l][src] % (PROGPOW_CACHE_BYTES / sizeof(uint32_t));
        mix[l][dst] = merge(mix[l][dst], dag[offset], sel);
      }
    }
    if (i < PROGPOW_CNT_MATH) {
      // Random Math
      // Generate 2 unique sources
      int src_rnd = kiss99(prog_rnd) % (PROGPOW_REGS * (PROGPOW_REGS - 1));
      int src1 = src_rnd % PROGPOW_REGS;  // 0 <= src1 < PROGPOW_REGS
      int src2 = src_rnd / PROGPOW_REGS;  // 0 <= src2 < PROGPOW_REGS - 1
      if (src2 >= src1) ++src2;           // src2 is now any reg other than src1
      int sel1 = kiss99(prog_rnd);
      int dst = mix_seq_dst[(mix_seq_dst_cnt++) % PROGPOW_REGS];
      int sel2 = kiss99(prog_rnd);
      for (int l = 0; l < PROGPOW_LANES; l++) {
        uint32_t data = math(mix[l][src1], mix[l][src2], sel1);
        mix[l][dst] = merge(mix[l][dst], data, sel2);
      }
    }
  }
  // Consume the global load data at the very end of the loop to allow full
  // latency hiding Always merge into mix[0] to feed the offset calculation
  for (int i = 0; i < PROGPOW_DAG_LOADS; i++) {
    int dst = (i == 0) ? 0 : mix_seq_dst[(mix_seq_dst_cnt++) % PROGPOW_REGS];
    int sel = kiss99(prog_rnd);
    for (int l = 0; l < PROGPOW_LANES; l++)
      mix[l][dst] = merge(mix[l][dst], dag_entry[l][i], sel);
  }
}

static inline uint64_t calcSeed(const hash32_t& seed_256) {
  // endian swap so byte 0 of the hash is the MSB of the value
  return uint64_t(vbk_swap_u32(seed_256.uint32s[0])) << 32 |
         vbk_swap_u32(seed_256.uint32s[1]);
}

hash32_t progPowHash(const uint64_t block_number,  // height
                     const uint64_t nonce,
                     const uint256& header,
                     const std::vector<uint32_t>& dag,
                     ethash_cache* light) {
  hash32_t digest, zero, seed_256;
  uint64_t seed = 0;
  uint32_t mix[PROGPOW_LANES][PROGPOW_REGS];

  // keccak(header..nonce)
  seed_256 = keccak_f800_progpow(header, nonce, digest);
  // extra 13 rounds of Keccakf800
  for (int i = 0; i < 13; i++) {
    seed = calcSeed(seed_256);
    seed_256 = keccak_f800_progpow(zero, seed, zero);
  }

  // VeriBlock: manually zero out the first bit of seed to reduce seed space to
  // 2^62
  seed = calcSeed(seed_256);
  seed &= 0x007FFFFFFFFFFFFFL;

  // initialize mix for all lanes
  for (int l = 0; l < PROGPOW_LANES; l++) {
    fill_mix(seed, l, {mix[l], PROGPOW_REGS});
  }

  // execute the randomly generated inner loop
  for (int i = 0; i < PROGPOW_CNT_DAG; i++) {
    progPowLoop(block_number, i, mix, dag, light);
  }

  // Reduce mix data to a per-lane 32-bit digest
  uint32_t digest_lane[PROGPOW_LANES];
  for (int l = 0; l < PROGPOW_LANES; l++) {
    digest_lane[l] = FNV_OFFSET_BASIS;
    for (int i = 0; i < PROGPOW_REGS; i++) {
      digest_lane[l] = fnv1a(digest_lane[l], mix[l][i]);
    }
  }
  // Reduce all lanes to a single 256-bit digest
  for (int i = 0; i < 8; i++) {
    digest.uint32s[i] = FNV_OFFSET_BASIS;
  }
  for (int l = 0; l < PROGPOW_LANES; l++) {
    digest.uint32s[l % 8] = fnv1a(digest.uint32s[l % 8], digest_lane[l]);
  }

  // keccak(header .. keccak(header..nonce) .. digest);
  return keccak_f800_progpow(header, seed, digest);
}

uint256 getVbkHeaderHash(Slice<const uint8_t> header) {
  // chop last 5 bytes (nonce)
  VBK_ASSERT(header.size() == VBK_HEADER_SIZE_PROGPOW);
  return sha256twice({header.begin(), header.size() - 5});
}

std::vector<uint32_t> createDagCache(ethash_cache* light) {
  std::vector<uint32_t> cdag(VBK_ETHASH_HASH_BYTES * DATASET_PARENTS, 0);

  ethash_dag_node_t node;
  for (size_t i = 0, total = cdag.size() / 16; i < total; ++i) {
    ethash_calculate_dag_node(&node, uint32_t(i), light);
    for (int j = 0; j < VBK_ETHASH_DAG_NODE_SIZE; j++) {
      cdag[i * 16 + j] = node.words[j];
    }
  }

  return cdag;
}

hash32_t hash32_t::readLE(ReadStream& rs) {
  hash32_t ret;
  ValidationState dummy;
  for (int i = 0; i < 8; i++) {
    ret.uint32s[i] = rs.assertReadLE<uint32_t>();
  }
  return ret;
}

hash32_t hash32_t::readBE(ReadStream& rs) {
  hash32_t ret;
  ValidationState dummy;
  for (int i = 0; i < 8; i++) {
    ret.uint32s[i] = rs.assertReadBE<uint32_t>();
  }
  return ret;
}

bool hash32_t::operator==(const hash32_t& h) const {
  for (int i = 0; i < 8; i++) {
    if (uint32s[i] != h.uint32s[i]) {
      return false;
    }
  }

  return true;
}

hash32_t::hash32_t() {
  for (int i = 0; i < 8; i++) {
    uint32s[i] = 0;
  }
}

std::string hash32_t::toHex() const {
  char* u = (char*)&uint32s[0];
  return HexStr(u, u + 32);
}
}  // namespace progpow

#ifndef VBK_PROGPOW_ETHASH_CACHE_SIZE
#define VBK_PROGPOW_ETHASH_CACHE_SIZE 6
#endif

#ifndef VBK_PROGPOW_HEADER_HASH_SIZE
#define VBK_PROGPOW_HEADER_HASH_SIZE 100000
#endif

using LockGuard = std::lock_guard<VBK_TRACE_LOCKABLE_BASE(std::mutex)>;

// epoch -> ethash cache + dag
struct EthashCache_t : public EthashCacheI {
  std::shared_ptr<CacheEntry> getOrDefault(
      uint64_t epoch,
      std::function<std::shared_ptr<CacheEntry>()> factory) override {
    return this->in_memory_cache.getOrDefault(epoch, factory);
  }

  void clear() override {
    this->in_memory_cache.clear();
    if (this->on_disk_cache != nullptr) {
      this->on_disk_cache->clear();
    }
  }

  void setOnDiskCache(const std::shared_ptr<EthashCache>& cache) {
    this->on_disk_cache = cache;
  }

 private:
  cache::SmallLFRUCache<uint64_t, CacheEntry, VBK_PROGPOW_ETHASH_CACHE_SIZE>
      in_memory_cache{};

  std::shared_ptr<EthashCacheI> on_disk_cache{nullptr};
};

static EthashCache_t ethash_cache{};

// protects EthashCache
static VBK_TRACE_LOCKABLE_BASE(std::mutex) & GetEthashCacheMutex() {
  static VBK_TRACE_LOCKABLE(std::mutex, csEthashCache);
  return csEthashCache;
}

void setEthashCache(const std::shared_ptr<EthashCache>& cache) {
  LockGuard lock(GetEthashCacheMutex());
  ethash_cache.setOnDiskCache(cache);
}

static EthashCacheI& GetEthashCache() {
  // NOLINTNEXTLINE(cert-err58-cpp)
  return ethash_cache;
}

// sha256d(vbkheader) -> progpow hash
struct ProgpowHeaderCache_T : public ProgpowHeaderCacheI {
  ProgpowHeaderCache_T(size_t maxSize, size_t elasticity)
      : in_memory_cache(maxSize, elasticity), on_disk_cache{nullptr} {}

  void insert(const uint256& key, uint192 value) override {
    if (this->on_disk_cache != nullptr) {
      return this->on_disk_cache->insert(key, value);
    }

    return this->in_memory_cache.insert(key, value);
  }

  bool tryGet(const uint256& key, uint192& value) override {
    if (this->on_disk_cache != nullptr) {
      return this->on_disk_cache->tryGet(key, value);
    }

    return this->in_memory_cache.tryGet(key, value);
  }

  void clear() override {
    if (this->on_disk_cache != nullptr) {
      return this->on_disk_cache->clear();
    }

    return this->in_memory_cache.clear();
  }

  void setOnDiskCache(const std::shared_ptr<ProgpowHeaderCache>& cache) {
    this->on_disk_cache = cache;
  }

 private:
  lru11::Cache<uint256, uint192, std::mutex> in_memory_cache;
  std::shared_ptr<ProgpowHeaderCacheI> on_disk_cache{nullptr};
};

static ProgpowHeaderCache_T progpow_header_cache(VBK_PROGPOW_HEADER_HASH_SIZE,
                                                 1000);

// protects ProgpowHeaderCache
static VBK_TRACE_LOCKABLE_BASE(std::mutex) & GetProgpowHeaderCacheMutex() {
  static VBK_TRACE_LOCKABLE(std::mutex, csProgpowHeaderCache);
  return csProgpowHeaderCache;
}

void setProgpowHeaderCache(const std::shared_ptr<ProgpowHeaderCache>& cache) {
  LockGuard lock(GetProgpowHeaderCacheMutex());
  progpow_header_cache.setOnDiskCache(cache);
}

// NOLINTNEXTLINE(cert-err58-cpp)
static ProgpowHeaderCacheI& GetProgpowHeaderCache() {
  return progpow_header_cache;
}

void progpow::insertHeaderCacheEntry(Slice<const uint8_t> header,
                                     uint192 progpowHash) {
  LockGuard lock(GetProgpowHeaderCacheMutex());
  VBK_ASSERT(header.size() == VBK_HEADER_SIZE_PROGPOW);
  auto hash = sha256twice(header);
  GetProgpowHeaderCache().insert(hash, std::move(progpowHash));
}

void progpow::clearHeaderCache() {
  LockGuard lock(GetProgpowHeaderCacheMutex());
  GetProgpowHeaderCache().clear();
}
void progpow::clearEthashCache() {
  LockGuard lock(GetEthashCacheMutex());
  GetEthashCache().clear();
}
static uint192 progPowHashImpl(Slice<const uint8_t> header) {
  VBK_ASSERT(header.size() == VBK_HEADER_SIZE_PROGPOW);
  const auto height = progpow::getVbkBlockHeight(header);
  const uint64_t epoch = progpow::ethashGetEpoch(height);
  const auto headerHash = progpow::getVbkHeaderHash(header);
  auto nonce = progpow::getVbkBlockNonce(header);

  // nonce is only 40 bits (5 bytes)
  nonce &= 0x000000FFFFFFFFFFLL;

  std::shared_ptr<CacheEntry> cacheEntry;
  {
    // cache miss
    LockGuard lock(GetEthashCacheMutex());
    cacheEntry = GetEthashCache().getOrDefault(epoch, [epoch, height] {
      VBK_LOG_WARN(
          "Calculating vProgPoW cache for epoch %d. Cache size=%d bytes.",
          epoch,
          progpow::ethash_get_cachesize(height));
      auto entry = std::make_shared<CacheEntry>();
      // build cache. this takes ~2.6sec
      entry->light = progpow::ethash_make_cache(height);
      entry->dag = progpow::createDagCache(entry->light.get());
      return entry;
    });
  }

  VBK_ASSERT(cacheEntry->light);
  auto& dag = cacheEntry->dag;
  auto* light = cacheEntry->light.get();
  auto hash = progpow::progPowHash(height, nonce, headerHash, dag, light);

  WriteStream w(32);
  std::for_each(hash.uint32s, hash.uint32s + 8, [&w](uint32_t item) {
    w.writeLE<uint32_t>(item);
  });

  auto& v = w.data();
  return {{v.data(), VBLAKE_HASH_SIZE}};
}

uint192 progPowHash(Slice<const uint8_t> header, progpow::ethash_cache* light) {
  VBK_ASSERT(header.size() == VBK_HEADER_SIZE_PROGPOW);
  const auto height = progpow::getVbkBlockHeight(header);
  const auto headerHash = progpow::getVbkHeaderHash(header);
  auto nonce = progpow::getVbkBlockNonce(header);

  // nonce is only 40 bits (5 bytes)
  nonce &= 0x000000FFFFFFFFFFLL;

  auto dag = progpow::createDagCache(light);
  auto hash = progpow::progPowHash(height, nonce, headerHash, dag, light);

  WriteStream w(32);
  std::for_each(hash.uint32s, hash.uint32s + 8, [&w](uint32_t item) {
    w.writeLE<uint32_t>(item);
  });

  auto& v = w.data();
  return {{v.data(), VBLAKE_HASH_SIZE}};
}

uint192 progPowHash(Slice<const uint8_t> header) {
  VBK_TRACE_ZONE_SCOPED_S(40);
  // replace very slow progpow hash with very fast sha256 hash
#if defined(VBK_FUZZING_UNSAFE_FOR_PRODUCTION)
  auto hash = sha256(header);
  return {{hash.data(), uint192::size()}};
#else
  // real impl
  VBK_ASSERT(header.size() == VBK_HEADER_SIZE_PROGPOW);
  const auto headerSha256 = sha256twice(header);
  uint192 ret{};
  {
    LockGuard lock(GetProgpowHeaderCacheMutex());
    if (GetProgpowHeaderCache().tryGet(headerSha256, ret)) {
      // cache hit
      return ret;
    }
  }
  // cache miss
  ret = progPowHashImpl(header);
  {
    LockGuard lock(GetProgpowHeaderCacheMutex());
    GetProgpowHeaderCache().insert(headerSha256, ret);
  }
  return ret;
#endif
}

}  // namespace altintegration
