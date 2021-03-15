// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ENDORSEMENT_H
#define VERIBLOCK_POP_CPP_ENDORSEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "serde.h"
#include "type_helpers.h"

// used for both: ALT endorsement and VBK endorsement
POP_DECLARE_ENTITY(endorsement);

POP_DECLARE_ARRAY(pop_endorsement_t, endorsement);

POP_ENTITY_GETTER(endorsement, pop_array_u8_t, id);
POP_ENTITY_GETTER(endorsement, pop_array_u8_t, endorsed_hash);
POP_ENTITY_GETTER(endorsement, pop_array_u8_t, containing_hash);
POP_ENTITY_GETTER(endorsement, pop_array_u8_t, block_of_proof);

POP_DECLARE_SERDE_VBK(endorsement);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_ENDORSEMENT_H
