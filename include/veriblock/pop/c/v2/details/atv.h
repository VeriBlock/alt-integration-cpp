// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ATV_H
#define VERIBLOCK_POP_CPP_ATV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "type_helpers.h"
#include "vbk_block.h"

POP_DECLARE_ENTITY(atv);

POP_ENTITY_GETTER(atv, uint32_t, version);
POP_ENTITY_GETTER(atv, pop_array_u8_t, id);
POP_ENTITY_GETTER(atv, pop_vbk_tx*, transaction);
POP_ENTITY_GETTER(atv, pop_merkle_path*, merkle_path);
POP_ENTITY_GETTER(atv, pop_vbk_block_t*, block_of_proof);

POP_DECLARE_SERDE_VBK(atv);

POP_DECLARE_ARRAY(pop_vtb_t, vtb);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_ATV_H
