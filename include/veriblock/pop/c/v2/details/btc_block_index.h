// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_H
#define VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "btc_block.h"
#include "endorsement.h"
#include "type_helpers.h"

POP_DECLARE_ENTITY(btc_block_index);

POP_ENTITY_GETTER(btc_block_index, int32_t, height);
POP_ENTITY_GETTER(btc_block_index, uint32_t, status);
POP_ENTITY_GETTER(btc_block_index, pop_array_u8_t, hash);
POP_ENTITY_GETTER(btc_block_index, pop_btc_block_index_t*, prev);
POP_ENTITY_GETTER(btc_block_index, pop_array_u32_t, refs);
POP_ENTITY_GETTER(btc_block_index,
                  pop_array_endorsement_t,
                  block_of_proof_endorsements);

POP_DECLARE_SERDE_VBK(btc_block_index);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_H
