// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ENTITIES_VBKBLOCK_H
#define VERIBLOCK_POP_CPP_ENTITIES_VBKBLOCK_H

#include "array.h"
#include "serde.h"
#include "type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(vbk_block);

POP_ENTITY_DECLARE_GETTER(vbk_block, POP_ARRAY_NAME(u8), hash);
POP_ENTITY_DECLARE_GETTER(vbk_block, POP_ARRAY_NAME(u8), previous_block);
POP_ENTITY_DECLARE_GETTER(vbk_block, POP_ARRAY_NAME(u8), merkle_root);
POP_ENTITY_DECLARE_GETTER(vbk_block, int16_t, version);
POP_ENTITY_DECLARE_GETTER(vbk_block, uint32_t, timestamp);
POP_ENTITY_DECLARE_GETTER(vbk_block, uint32_t, difficulty);
POP_ENTITY_DECLARE_GETTER(vbk_block, uint64_t, nonce);
POP_ENTITY_DECLARE_GETTER(vbk_block, int32_t, height);

POP_DECLARE_ARRAY(POP_ENTITY_NAME(vbk_block) *, vbk_block);

POP_GENERATE_DEFAULT_VALUE(vbk_block);

#ifdef __cplusplus
}
#endif

#endif