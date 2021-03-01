// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BTC_BLOCK_H
#define VERIBLOCK_POP_CPP_BTC_BLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "serde.h"

typedef struct __pop_btc_block pop_btc_block_t;

void pop_btc_block_free(const pop_btc_block_t* self);

const pop_array_u8_t* pop_btc_block_get_hash(pop_btc_block_t* self);
uint32_t pop_btc_block_get_version(pop_btc_block_t* self);
const pop_array_u8_t* pop_btc_block_get_previous_block(pop_btc_block_t* self);
const pop_array_u8_t* pop_btc_block_get_merkle_root(pop_btc_block_t* self);
uint32_t pop_btc_block_get_timestamp(pop_btc_block_t* self);
uint32_t pop_btc_block_get_difficulty(pop_btc_block_t* self);
uint32_t pop_btc_block_get_nonce(pop_btc_block_t* self);

POP_DECLARE_SERDE_VBK(btc_block, pop_btc_block_t);
POP_DECLARE_SERDE_RAW(btc_block, pop_btc_block_t);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_BTC_BLOCK_H
