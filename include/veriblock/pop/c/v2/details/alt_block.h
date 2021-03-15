// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ALT_BLOCK_H
#define VERIBLOCK_POP_CPP_ALT_BLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "serde.h"

typedef struct __pop_alt_block pop_alt_block_t;

const pop_array_u8_t* pop_alt_block_get_height(const pop_alt_block_t* self);
const pop_array_u8_t* pop_alt_block_get_previous_block(
    const pop_alt_block_t* self);
uint32_t pop_alt_block_get_timestamp(const pop_alt_block_t* self);
const pop_array_u8_t* pop_alt_block_get_hash(const pop_alt_block_t* self);

POP_DECLARE_SERDE_VBK(pop_alt_block_t);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_ALT_BLOCK_H
