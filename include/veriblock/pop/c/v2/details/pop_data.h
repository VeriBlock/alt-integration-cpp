// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_POP_DATA_H
#define VERIBLOCK_POP_CPP_POP_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "atv.h"
#include "serde.h"
#include "type_helpers.h"
#include "vbk_block.h"
#include "vtb.h"

POP_DECLARE_ENTITY(pop_data);

POP_ENTITY_GETTER(pop_data, uint32_t, version);
POP_ENTITY_GETTER(pop_data, pop_array_vbk_block_t, context);
POP_ENTITY_GETTER(pop_data, pop_array_vtb_t, vtbs);
POP_ENTITY_GETTER(pop_data, pop_array_atv_t, atvs);

POP_ENTITY_GETTER_ALLOC(pop_data, pop_array_u8_t, merkle_root);

POP_DECLARE_SERDE_VBK(pop_data);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_POP_DATA_H
