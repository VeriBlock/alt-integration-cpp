// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

// A simple vblake Reference Implementation.

#include <veriblock/pop/crypto/vblake.hpp>
#include <cstdint>
#include <cstdio>

#ifndef VBK_COUNTOF
#define VBK_COUNTOF(x) (sizeof(x) / sizeof((x)[0]))
#endif

namespace altintegration {
//==========================================================================================
//                      CONSTS
//==========================================================================================

static const uint64_t cnt_vblake_iv = 8;
static const uint64_t vblake_iv[cnt_vblake_iv] = {0x4BBF42C1F006AD9DL,
                                                  0x5D11A8C3B5AEB12EL,
                                                  0xA64AB78DC2774652L,
                                                  0xC67595724658F253L,
                                                  0xB8864E79CB891E56L,
                                                  0x12ED593E29FB41A1L,
                                                  0xB1DA3AB63C60BAA8L,
                                                  0x6D20E50C1F954DEDL};

static const uint8_t sigma[16][16] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
    {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
    {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
    {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
    {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
    {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11},
    {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
    {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5},
    {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
    {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
    {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
    {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
    {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9}};

static const uint64_t vblake_c[] = {0xA51B6A89D489E800L,
                                    0xD35B2E0E0B723800L,
                                    0xA47B39A2AE9F9000L,
                                    0x0C0EFA33E77E6488L,
                                    0x4F452FEC309911EBL,
                                    0x3CFCC66F74E1022CL,
                                    0x4606AD364DC879DDL,
                                    0xBBA055B53D47C800L,
                                    0x531655D90C59EB1BL,
                                    0xD1A00BA6DAE5B800L,
                                    0x2FE452DA9632463EL,
                                    0x98A7B5496226F800L,
                                    0xBAFCD004F92CA000L,
                                    0x64A39957839525E7L,
                                    0xD859E6F081AAE000L,
                                    0x63D980597B560E6BL};

//==========================================================================================
//                                  TOOLS
//==========================================================================================

/**
 * Rotate x, a 64-bit long, to the right by y places
 */
static inline uint64_t vblake_ROTR64(uint64_t x, uint64_t y) {
  return (((x) >> (y)) ^ ((x) << (64 - (y))));
}

//
// The G Mixing function from the Blake2 specification.
//
static void vblake_B2B_G(uint64_t *v,
                         uint32_t a,
                         uint32_t b,
                         uint32_t c,
                         uint32_t d,
                         uint64_t x,
                         uint64_t y,
                         uint64_t c1,
                         uint64_t c2) {
  v[a] = v[a] + v[b] + (x ^ c1);
  v[d] ^= v[a];
  v[d] = vblake_ROTR64(v[d], 60);
  v[c] = v[c] + v[d];
  v[b] = vblake_ROTR64(v[b] ^ v[c], 43);
  v[a] = v[a] + v[b] + (y ^ c2);
  v[d] = vblake_ROTR64(v[d] ^ v[a], 5);
  v[c] = v[c] + v[d];
  v[b] = vblake_ROTR64(v[b] ^ v[c], 18);

  // X'Y'Z' + X'YZ + XY'Z + XYZ'    LUT: 10010110
  v[d] ^= (~v[a] & ~v[b] & ~v[c]) | (~v[a] & v[b] & v[c]) |
          (v[a] & ~v[b] & v[c]) | (v[a] & v[b] & ~v[c]);

  // X'Y'Z + X'YZ' + XY'Z' + XYZ    LUT: 01101001
  v[d] ^= (~v[a] & ~v[b] & v[c]) | (~v[a] & v[b] & ~v[c]) |
          (v[a] & ~v[b] & ~v[c]) | (v[a] & v[b] & v[c]);
}

//==========================================================================================
//                       WORK UNITS
//==========================================================================================

//
// Compression function. "last" flag indicates last block.
//
static void vblake_compress(vblake_ctx *ctx) {
  uint64_t i = 0;
  static const uint64_t kNumOfRounds = 16;
  uint64_t v[kNumOfRounds] = {};
  uint64_t m[kNumOfRounds] = {};

  // init work variables
  for (i = 0; i < 8; i++) {
    v[i] = ctx->h[i];
    v[i + 8] = vblake_iv[i];
  }

  v[12] ^= 64;          // Input count low
  v[13] ^= 0;           // Input count high (no overflow, therefore 0)
  v[14] ^= (long)(-1);  // f[0] = 0xFF..FF
  v[15] ^= 0;           // f[1] = 0x00..00

  for (i = 0; i < 8; i++) {
    m[i] = ((uint64_t)(ctx->b[i * 8u + 7u] & 0xFFu) << 56u) ^
           ((uint64_t)(ctx->b[i * 8u + 6u] & 0xFFu) << 48u) ^
           ((uint64_t)(ctx->b[i * 8u + 5u] & 0xFFu) << 40u) ^
           ((uint64_t)(ctx->b[i * 8u + 4u] & 0xFFu) << 32u) ^
           ((uint64_t)(ctx->b[i * 8u + 3u] & 0xFFu) << 24u) ^
           ((uint64_t)(ctx->b[i * 8u + 2u] & 0xFFu) << 16u) ^
           ((uint64_t)(ctx->b[i * 8u + 1u] & 0xFFu) << 8u) ^
           ((uint64_t)(ctx->b[i * 8u + 0u] & 0xFFu) << 0u);
  }

  // Using 16 rounds of the Blake2 G function, drawing on the additional 4 rows
  // of sigma from reference BLAKE implementation

  for (i = 0; i < kNumOfRounds; i++) {
    vblake_B2B_G(v,
                 0,
                 4,
                 8,
                 12,
                 m[sigma[i][1]],
                 m[sigma[i][0]],
                 vblake_c[sigma[i][1]],
                 vblake_c[sigma[i][0]]);
    vblake_B2B_G(v,
                 1,
                 5,
                 9,
                 13,
                 m[sigma[i][3]],
                 m[sigma[i][2]],
                 vblake_c[sigma[i][3]],
                 vblake_c[sigma[i][2]]);
    vblake_B2B_G(v,
                 2,
                 6,
                 10,
                 14,
                 m[sigma[i][5]],
                 m[sigma[i][4]],
                 vblake_c[sigma[i][5]],
                 vblake_c[sigma[i][4]]);
    vblake_B2B_G(v,
                 3,
                 7,
                 11,
                 15,
                 m[sigma[i][7]],
                 m[sigma[i][6]],
                 vblake_c[sigma[i][7]],
                 vblake_c[sigma[i][6]]);
    vblake_B2B_G(v,
                 0,
                 5,
                 10,
                 15,
                 m[sigma[i][9]],
                 m[sigma[i][8]],
                 vblake_c[sigma[i][9]],
                 vblake_c[sigma[i][8]]);
    vblake_B2B_G(v,
                 1,
                 6,
                 11,
                 12,
                 m[sigma[i][11]],
                 m[sigma[i][10]],
                 vblake_c[sigma[i][11]],
                 vblake_c[sigma[i][10]]);
    vblake_B2B_G(v,
                 2,
                 7,
                 8,
                 13,
                 m[sigma[i][13]],
                 m[sigma[i][12]],
                 vblake_c[sigma[i][13]],
                 vblake_c[sigma[i][12]]);
    vblake_B2B_G(v,
                 3,
                 4,
                 9,
                 14,
                 m[sigma[i][15]],
                 m[sigma[i][14]],
                 vblake_c[sigma[i][15]],
                 vblake_c[sigma[i][14]]);
  }

  // Update h[0 .. 7]
  for (i = 0; i < 8; ++i) {
    ctx->h[i] ^= v[i] ^ v[i + 8];
  }

  ctx->h[0] ^= ctx->h[3] ^ ctx->h[6];
  ctx->h[1] ^= ctx->h[4] ^ ctx->h[7];
  ctx->h[2] ^= ctx->h[5];
}

static inline void vblake_create_ctx(vblake_ctx *ctx) {
  uint64_t i = 0;

  // state, "param block"
  for (i = 0; i < VBK_COUNTOF(ctx->h); i++) {
    ctx->h[i] = vblake_iv[i];
  }
  ctx->h[0] ^= 0x01010000u ^ VBLAKE_HASH_SIZE;

  // zero input block
  for (i = 0; i < VBK_COUNTOF(ctx->b); i++) {
    ctx->b[i] = 0;
  }
}

static void vblake_recombineB2Bh(vblake_ctx *ctx, void *out) {
  auto *output = (uint8_t *)out;
  for (int i = 0; i < 3; i++) {
    output[i * 8u + 0u] = (ctx->h[i] >> 0u) & 0xFFu;
    output[i * 8u + 1u] = (ctx->h[i] >> 8u) & 0xFFu;
    output[i * 8u + 2u] = (ctx->h[i] >> 16u) & 0xFFu;
    output[i * 8u + 3u] = (ctx->h[i] >> 24u) & 0xFFu;
    output[i * 8u + 4u] = (ctx->h[i] >> 32u) & 0xFFu;
    output[i * 8u + 5u] = (ctx->h[i] >> 40u) & 0xFFu;
    output[i * 8u + 6u] = (ctx->h[i] >> 48u) & 0xFFu;
    output[i * 8u + 7u] = (ctx->h[i] >> 56u) & 0xFFu;
  }
}

//==========================================================================================
//                      C INTERFACES
//==========================================================================================
//
// Initialize the hashing context "ctx"
//
void vblake_init(vblake_ctx *ctx) { vblake_create_ctx(ctx); }

//
// Add "inlen" bytes from "in" into the hash.
//
int vblake_update(vblake_ctx *ctx, const void *in, size_t inlen)  // data bytes
{
  if (inlen > 64) {
    return -1;
  }

  auto *input = (uint8_t *)in;
  for (size_t i = 0; i < inlen; ++i) {
    ctx->b[i] = input[i];
  }
  vblake_compress(ctx);
  return 0;
}

//
// Generate the message digest (size given in init).
//      Result placed in "out".
//
void vblake_final(vblake_ctx *ctx, void *out) {
  vblake_recombineB2Bh(ctx, out);
}

//
// Convenience function for all-in-one computation.
//
int vblake(void *out, const void *in, size_t inlen) {
  vblake_ctx ctx;
  vblake_init(&ctx);
  if (-1 == vblake_update(&ctx, in, inlen)) {
    return -1;
  }
  vblake_final(&ctx, out);
  return 0;
}

}  // namespace altintegration
