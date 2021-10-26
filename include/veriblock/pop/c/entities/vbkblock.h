// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_VBKBLOCK_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_VBKBLOCK_H

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/serde.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(vbk_block);

POP_ENTITY_GETTER_FUNCTION(vbk_block, POP_ARRAY_NAME(u8), id);
POP_ENTITY_GETTER_FUNCTION(vbk_block, POP_ARRAY_NAME(u8), hash);
POP_ENTITY_GETTER_FUNCTION(vbk_block, POP_ARRAY_NAME(u8), previous_block);
POP_ENTITY_GETTER_FUNCTION(vbk_block, POP_ARRAY_NAME(u8), merkle_root);
POP_ENTITY_GETTER_FUNCTION(vbk_block, POP_ARRAY_NAME(u8), previous_keystone);
POP_ENTITY_GETTER_FUNCTION(vbk_block,
                           POP_ARRAY_NAME(u8),
                           second_previous_keystone);
POP_ENTITY_GETTER_FUNCTION(vbk_block, int16_t, version);
POP_ENTITY_GETTER_FUNCTION(vbk_block, uint32_t, timestamp);
POP_ENTITY_GETTER_FUNCTION(vbk_block, int32_t, difficulty);
POP_ENTITY_GETTER_FUNCTION(vbk_block, uint64_t, nonce);
POP_ENTITY_GETTER_FUNCTION(vbk_block, int32_t, height);

POP_ENTITY_TO_JSON(vbk_block);

POP_ENTITY_SERIALIZE_TO_VBK(vbk_block);
POP_ENTITY_DESERIALIZE_FROM_VBK(vbk_block);

POP_DECLARE_ARRAY(POP_ENTITY_NAME(vbk_block) *, vbk_block);

POP_GENERATE_DEFAULT_VALUE(vbk_block);

#ifdef __cplusplus
}
#endif

#endif