// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <cstdint>
#include <vector>
#include <veriblock/consts.hpp>
#include <veriblock/crypto/progpow.hpp>
#include <veriblock/crypto/progpow/math.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/hashutil.hpp>
#include <veriblock/serde.hpp>
#include <veriblock/slice.hpp>

#include "libethash/internal.hpp"
#include "veriblock/crypto/progpow/ethash.hpp"
#include "veriblock/crypto/progpow/kiss99.hpp"

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
  }

  VBK_ASSERT(false);
  return 0;
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
  }
  // clang-format on

  VBK_ASSERT(false);
  return 0;
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

// Implementation of the Keccakf transformation with a width of 800
void keccak_f800_round(Slice<uint32_t> st, const int r) {
  static const uint32_t keccakf_rotc[24] = {1,  3,  6,  10, 15, 21, 28, 36,
                                            45, 55, 2,  14, 27, 41, 56, 8,
                                            25, 43, 62, 18, 39, 61, 20, 44};
  static const uint32_t keccakf_piln[24] = {10, 7,  11, 17, 18, 3,  5,  16,
                                            8,  21, 24, 4,  15, 23, 19, 13,
                                            12, 2,  20, 14, 22, 9,  6,  1};

  uint32_t t, bc[5];
  // VeriBlock: Theta
  bc[0] = st[0] ^ st[6] ^ st[9] ^ st[12] ^ st[17];
  bc[1] = st[8] ^ st[11] ^ st[14] ^ st[19] ^ st[23];
  bc[2] = st[2] ^ st[7] ^ st[10] ^ st[18] ^ st[22];
  bc[3] = st[4] ^ st[5] ^ st[15] ^ st[20] ^ st[24];
  bc[4] = st[1] ^ st[3] ^ st[13] ^ st[16] ^ st[21];

  for (int i = 0; i < 5; i++) {
    t = bc[(i + 4) % 5] ^ rotl32(bc[(i + 1) % 5], 1u);
    for (uint32_t j = 0; j < 25; j += 5) {
      st[j + i] ^= t;
    }
  }

  // Rho Pi
  t = st[1];
  for (int i = 0; i < 24; i++) {
    uint32_t j = keccakf_piln[i];
    bc[0] = st[j];
    st[j] = rotl32(t, keccakf_rotc[i]);
    t = bc[0];
  }

  // VeriBlock: update
  st[3] = st[3] ^ 0x79938B61;
  st[10] = st[10] ^ ((st[19] & 0x000000FF) | (st[24] & 0x0000FF00) |
                     (st[6] & 0x00FF0000) | (st[14] & 0xFF000000));

  //  Chi
  for (uint32_t j = 0; j < 25; j += 5) {
    for (int i = 0; i < 5; i++) bc[i] = st[j + i];
    for (int i = 0; i < 5; i++)
      st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
  }

  //  Iota
  st[0] ^= keccakf_rndc[r];
}

void keccak_f800(Slice<uint32_t> state) {
  for (int r = 0; r < 22; r++) {
    keccak_f800_round(state, r);
  }
}

hash32_t keccak_f800_progpow(const uint256& header,
                             uint64_t seed,
                             const hash32_t& digest) {
  hash32_t hdr;
  ReadStream rh(header);
  for (int i = 0; i < 8; i++) {
    hdr.uint32s[i] = rh.readLE<uint32_t>();
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
  for (int i = PROGPOW_REGS - 1, j; i > 0; i--) {
    j = kiss99(prog_rnd) % (i + 1);
    std::swap(mix_seq_dst[i], mix_seq_dst[j]);
    j = kiss99(prog_rnd) % (i + 1);
    std::swap(mix_seq_src[i], mix_seq_src[j]);
  }
  return prog_rnd;
}

void progPowLoop(const uint64_t block_number,
                 const uint32_t loop,
                 uint32_t mix[PROGPOW_LANES][PROGPOW_REGS],
                 const std::vector<uint32_t>& dag,
                 ethash_light_t light) {
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

  uint32_t lastLookup = (uint32_t)-1;
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
                     ethash_light_t light) {
  hash32_t digest, zero, seed_256;
  uint64_t seed;
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

std::vector<uint32_t> createDagCache(ethash_light_t light) {
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
  for (int i = 0; i < 8; i++) {
    ret.uint32s[i] = rs.readLE<uint32_t>();
  }
  return ret;
}

hash32_t hash32_t::readBE(ReadStream& rs) {
  hash32_t ret;
  for (int i = 0; i < 8; i++) {
    ret.uint32s[i] = rs.readBE<uint32_t>();
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

uint192 progPowHash(Slice<const uint8_t> header) {
  VBK_ASSERT(header.size() == VBK_HEADER_SIZE_PROGPOW);
  const auto height = progpow::getVbkBlockHeight(header);
  const auto headerHash = progpow::getVbkHeaderHash(header);
  auto nonce = progpow::getVbkBlockNonce(header);

  // nonce is only 40 bits (5 bytes)
  nonce &= 0x000000FFFFFFFFFFLL;

  // build cache
  std::shared_ptr<progpow::ethash_light> light(
      progpow::ethash_light_new(height), progpow::ethash_light_delete);

  auto dag = progpow::createDagCache(light.get());
  auto hash = progpow::progPowHash(height, nonce, headerHash, dag, light.get());

  WriteStream w(32);
  std::for_each(hash.uint32s, hash.uint32s + 8, [&w](uint32_t item) {
    w.writeLE<uint32_t>(item);
  });

  auto& v = w.data();
  return uint192({v.data(), VBLAKE_HASH_SIZE});
}

}  // namespace altintegration