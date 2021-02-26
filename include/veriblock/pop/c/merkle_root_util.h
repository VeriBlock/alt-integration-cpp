// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_MERKLE_ROOT_UTIL_H
#define VERIBLOCK_POP_CPP_MERKLE_ROOT_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define POP_SHA256_SIZE 32

typedef uint8_t pop_uint256[POP_SHA256_SIZE];

//! IMPLEMENT THIS IN USERLAND CODE
extern void pop_sha256d(pop_uint256 output, const void* input, int size);

inline void pop_sha256d_concat(pop_uint256 output,
                               const pop_uint256 left,
                               const pop_uint256 right) {
  const int size = POP_SHA256_SIZE * 2;
  uint8_t bytes[size];
  memcpy(bytes, left, POP_SHA256_SIZE);
  memcpy(bytes + POP_SHA256_SIZE, right, POP_SHA256_SIZE);
  pop_sha256d(output, bytes, size);
}

typedef struct pop_context_info {
  int height;
  pop_uint256 first_prev_keystone;
  pop_uint256 second_prev_keystone;
} pop_context_info;

inline pop_context_info pop_create_context_info(int height,
                                                const pop_uint256 first,
                                                const pop_uint256 second) {
  pop_context_info r;
  r.height = height;
  memcpy(r.first_prev_keystone, first, POP_SHA256_SIZE);
  memcpy(r.second_prev_keystone, second, POP_SHA256_SIZE);
  return r;
}

inline uint8_t* pop_write_int_bigendian(uint8_t* out, int value) {
  assert(out);
  assert(sizeof(value) == 4);
  out[0] = value >> 24;
  out[1] = value >> 16;
  out[2] = value >> 8;
  out[3] = value;
  return out + 4;
}

inline uint8_t* pop_write_keystone(uint8_t* out, const pop_uint256 keystone) {
  // write size
  *(out++) = POP_SHA256_SIZE;
  // write keystone
  memcpy(out, keystone, POP_SHA256_SIZE);
  return out + POP_SHA256_SIZE;
}

inline void pop_context_info_hash(pop_uint256 out_ctx_hash,
                                  const pop_context_info* inctx) {
  static const int size = 4 + 1 + POP_SHA256_SIZE + 1 + POP_SHA256_SIZE;
  uint8_t bytes[size];
  memset(bytes, 0, size);
  uint8_t* ptr = bytes;
  ptr = pop_write_int_bigendian(ptr, inctx->height);
  ptr = pop_write_keystone(ptr, inctx->first_prev_keystone);
  /* */ pop_write_keystone(ptr, inctx->second_prev_keystone);
  pop_sha256d(out_ctx_hash, bytes, size);
}

inline void pop_calculate_top_level_merkle_root(
    pop_uint256 top_level_merkle_root,
    const pop_uint256 tx_root,
    const pop_uint256 pop_data_root,
    const pop_context_info context) {
  // calculate context info hash
  pop_uint256 context_hash;
  pop_context_info_hash(context_hash, &context);

  // combine tx_root with pop_data_root to get state_root
  pop_uint256 state_root;
  pop_sha256d_concat(state_root, tx_root, pop_data_root);

  // combine state_root and context_info_hash to get top_level_merkle_root
  pop_sha256d_concat(top_level_merkle_root, state_root, context_hash);
}

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_MERKLE_ROOT_UTIL_H
