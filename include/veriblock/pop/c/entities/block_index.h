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

// BlockValidityStatus
extern const uint32_t BLOCK_VALID_UNKNOWN;
extern const uint32_t BLOCK_BOOTSTRAP;
extern const uint32_t BLOCK_FAILED_BLOCK;
extern const uint32_t BLOCK_FAILED_POP;
extern const uint32_t BLOCK_FAILED_CHILD;
extern const uint32_t BLOCK_FAILED_MASK;
extern const uint32_t BLOCK_HAS_PAYLOADS;
extern const uint32_t BLOCK_ACTIVE;
extern const uint32_t BLOCK_DELETED;

POP_DECLARE_ENTITY(alt_block_index);
POP_DECLARE_ENTITY(vbk_block_index);
POP_DECLARE_ENTITY(btc_block_index);

//! get_header
POP_ENTITY_GETTER_FUNCTION(alt_block_index,
                           POP_ENTITY_NAME(alt_block) *,
                           header);
POP_ENTITY_GETTER_FUNCTION(vbk_block_index,
                           POP_ENTITY_NAME(vbk_block) *,
                           header);
POP_ENTITY_GETTER_FUNCTION(btc_block_index,
                           POP_ENTITY_NAME(btc_block) *,
                           header);
//! get_status
POP_ENTITY_GETTER_FUNCTION(alt_block_index, uint32_t, status);
POP_ENTITY_GETTER_FUNCTION(vbk_block_index, uint32_t, status);
POP_ENTITY_GETTER_FUNCTION(btc_block_index, uint32_t, status);

//! get_height
POP_ENTITY_GETTER_FUNCTION(alt_block_index, uint32_t, height);
POP_ENTITY_GETTER_FUNCTION(vbk_block_index, uint32_t, height);
POP_ENTITY_GETTER_FUNCTION(btc_block_index, uint32_t, height);

//! has_flag
POP_ENTITY_CUSTOM_FUNCTION(alt_block_index, bool, has_flag, uint32_t flag);
POP_ENTITY_CUSTOM_FUNCTION(vbk_block_index, bool, has_flag, uint32_t flag);
POP_ENTITY_CUSTOM_FUNCTION(btc_block_index, bool, has_flag, uint32_t flag);

//! toJSON
POP_ENTITY_TO_JSON(alt_block_index);
POP_ENTITY_TO_JSON(vbk_block_index);
POP_ENTITY_TO_JSON(btc_block_index);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif