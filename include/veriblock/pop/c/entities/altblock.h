// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_ALTBLOCK_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_ALTBLOCK_H

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/serde.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(alt_block);

POP_ENTITY_GETTER_FUNCTION(alt_block, POP_ARRAY_NAME(u8), hash);
POP_ENTITY_GETTER_FUNCTION(alt_block, POP_ARRAY_NAME(u8), previous_block);
POP_ENTITY_GETTER_FUNCTION(alt_block, uint32_t, timestamp);
POP_ENTITY_GETTER_FUNCTION(alt_block, int32_t, height);

POP_GENERATE_DEFAULT_VALUE(alt_block);

#ifdef __cplusplus
}
#endif

#endif