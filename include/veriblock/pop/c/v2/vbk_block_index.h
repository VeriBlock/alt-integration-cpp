// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_H
#define VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "block_index_common.h"
#include "vbk_block.h"

typedef struct __pop_vbk_block_index pop_vbk_block_index_t;

POP_DECLARE_BLOCK_INDEX_COMMON(vbk, pop_vbk_block_index_t, pop_vbk_block_t);

const pop_array_u8_t* pop_vbk_block_get_chainwork(
    const pop_vbk_block_index_t* self);
uint32_t pop_vbk_block_index_get_ref_count(const pop_vbk_block_index_t* self);

// TODO: get_containing_vtbs
// TODO: get_block_of_proof_endorsements

POP_DECLARE_SERDE_VBK(vbk_block_index, pop_vbk_block_index_t);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_H
