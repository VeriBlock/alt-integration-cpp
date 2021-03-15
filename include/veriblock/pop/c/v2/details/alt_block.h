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
#include "type_helpers.h"

POP_DECLARE_ENTITY(alt_block);

POP_ENTITY_GETTER(alt_block, uint32_t, timestamp);
POP_ENTITY_GETTER(alt_block, int32_t, height);
POP_ENTITY_GETTER(alt_block, pop_array_u8_t, previous_block);
POP_ENTITY_GETTER(alt_block, pop_array_u8_t, hash);

POP_DECLARE_SERDE_VBK(pop_alt_block_t);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_ALT_BLOCK_H
