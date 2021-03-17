// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ENTITIES_BTCBLOCK_H
#define VERIBLOCK_POP_CPP_ENTITIES_BTCBLOCK_H

#include "array.h"
#include "serde.h"
#include "type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(btc_block);

POP_ENTITY_GETTER_DECLARE(btc_block, POP_ARRAY_NAME(u8), hash);
POP_ENTITY_GETTER_DECLARE(btc_block, POP_ARRAY_NAME(u8), previous_block);
POP_ENTITY_GETTER_DECLARE(btc_block, POP_ARRAY_NAME(u8), merkle_root);
POP_ENTITY_GETTER_DECLARE(btc_block, uint32_t, version);
POP_ENTITY_GETTER_DECLARE(btc_block, uint32_t, timestamp);
POP_ENTITY_GETTER_DECLARE(btc_block, uint32_t, difficulty);
POP_ENTITY_GETTER_DECLARE(btc_block, uint32_t, nonce);

POP_DECLARE_ARRAY(pop_btc_block_t, btc_block);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_BTC_BLOCK_H