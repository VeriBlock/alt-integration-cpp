// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BTC_BLOCK_H
#define VERIBLOCK_POP_CPP_BTC_BLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "serde.h"
#include "type_helpers.h"

POP_DECLARE_ENTITY(btc_block);

POP_ENTITY_GETTER(btc_block, pop_array_u8_t, hash);
POP_ENTITY_GETTER(btc_block, pop_array_u8_t, previous_block);
POP_ENTITY_GETTER(btc_block, pop_array_u8_t, merkle_root);
POP_ENTITY_GETTER(btc_block, uint32_t, version);
POP_ENTITY_GETTER(btc_block, uint32_t, timestamp);
POP_ENTITY_GETTER(btc_block, uint32_t, difficulty);
POP_ENTITY_GETTER(btc_block, uint32_t, nonce);

POP_DECLARE_SERDE_VBK(btc_block);
POP_DECLARE_SERDE_RAW(btc_block);

POP_DECLARE_ARRAY(pop_btc_block_t, btc_block);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_BTC_BLOCK_H
