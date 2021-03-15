// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_BLOCK_H
#define VERIBLOCK_POP_CPP_VBK_BLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "serde.h"
#include "type_helpers.h"

POP_DECLARE_ENTITY(vbk_block);

POP_ENTITY_GETTER(vbk_block, pop_array_u8_t, previous_block);
POP_ENTITY_GETTER(vbk_block, pop_array_u8_t, previous_keystone);
POP_ENTITY_GETTER(vbk_block, pop_array_u8_t, second_previous_keystone);
POP_ENTITY_GETTER(vbk_block, pop_array_u8_t, merkle_root);
POP_ENTITY_GETTER(vbk_block, pop_array_u8_t, hash);
POP_ENTITY_GETTER(vbk_block, pop_array_u8_t, id);
POP_ENTITY_GETTER(vbk_block, int32_t, height);
POP_ENTITY_GETTER(vbk_block, uint32_t, version);
POP_ENTITY_GETTER(vbk_block, uint32_t, timestamp);
POP_ENTITY_GETTER(vbk_block, uint32_t, difficulty);
POP_ENTITY_GETTER(vbk_block, uint32_t, nonce);

POP_DECLARE_SERDE_VBK(vbk_block);
POP_DECLARE_SERDE_RAW(vbk_block);

POP_DECLARE_ARRAY(pop_vbk_block_t, vbk_block);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_VBK_BLOCK_H
