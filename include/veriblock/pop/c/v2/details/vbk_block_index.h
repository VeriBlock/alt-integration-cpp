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
#include "type_helpers.h"
#include "vbk_block.h"
#include "vtb.h"

POP_DECLARE_ENTITY(vbk_block_index);

POP_ENTITY_GETTER(vbk_block_index, int32_t, height);
POP_ENTITY_GETTER(vbk_block_index, uint32_t, status);
POP_ENTITY_GETTER(vbk_block_index, pop_array_u8_t, hash);
POP_ENTITY_GETTER(vbk_block_index, pop_vbk_block_index_t*, prev);
POP_ENTITY_GETTER(vbk_block_index, uint32_t, ref_count);
POP_ENTITY_GETTER(vbk_block_index, pop_array_u8_t, chainwork);
POP_ENTITY_GETTER(vbk_block_index, pop_array_vtb_t, containing_vtbs);
POP_ENTITY_GETTER(vbk_block_index,
                  pop_array_endorsement_t,
                  block_of_proof_endorsements);

POP_DECLARE_SERDE_VBK(vbk_block_index);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_H
