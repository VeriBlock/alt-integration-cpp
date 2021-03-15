// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VTB_H
#define VERIBLOCK_POP_CPP_VTB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "serde.h"
#include "type_helpers.h"
#include "vbk_pop_tx.h"

POP_DECLARE_ENTITY(vtb);

POP_ENTITY_GETTER(vtb, uint32_t, version);
POP_ENTITY_GETTER(vtb, pop_array_u8_t, id);
POP_ENTITY_GETTER(vtb, pop_vbk_pop_tx_t *, transaction);
POP_ENTITY_GETTER(vtb, pop_merkle_path_t *, merkle_path);
POP_ENTITY_GETTER(vtb, pop_vbk_block_t *, containing_block);

POP_DECLARE_SERDE_VBK(vtb);

POP_DECLARE_ARRAY(pop_vtb_t, vtb);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_VTB_H
