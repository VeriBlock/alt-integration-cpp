// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_BLOCK_INDEX_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_BLOCK_INDEX_H

#include <stdint.h>

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/altblock.h"
#include "veriblock/pop/c/entities/btcblock.h"
#include "veriblock/pop/c/entities/serde.h"
#include "veriblock/pop/c/entities/vbkblock.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(alt_block_index);
POP_DECLARE_ENTITY(vbk_block_index);
POP_DECLARE_ENTITY(btc_block_index);

//! get_header
POP_ENTITY_GETTER_FUNCTION(alt_block_index,
                           POP_ENTITY_NAME(alt_block) *,
                           get_header);
POP_ENTITY_GETTER_FUNCTION(vbk_block_index,
                           POP_ENTITY_NAME(vbk_block) *,
                           get_header);
POP_ENTITY_GETTER_FUNCTION(btc_block_index,
                           POP_ENTITY_NAME(btc_block) *,
                           get_header);
//! get_status
POP_ENTITY_GETTER_FUNCTION(alt_block_index, uint32_t, get_status);
POP_ENTITY_GETTER_FUNCTION(vbk_block_index, uint32_t, get_status);
POP_ENTITY_GETTER_FUNCTION(btc_block_index, uint32_t, get_status);

//! toJSON
POP_ENTITY_TO_JSON(alt_block_index);
POP_ENTITY_TO_JSON(vbk_block_index);
POP_ENTITY_TO_JSON(btc_block_index);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif