// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_POP_TX_H
#define VERIBLOCK_POP_CPP_VBK_POP_TX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "btc_block.h"
#include "merkle_path.h"
#include "serde.h"
#include "type_helpers.h"
#include "vbk_block.h"

POP_DECLARE_ENTITY(vbk_pop_tx);

POP_ENTITY_GETTER(vbk_pop_tx, bool, is_mainnet);
POP_ENTITY_GETTER(vbk_pop_tx, uint8_t, network_byte);
POP_ENTITY_GETTER(vbk_pop_tx, const char*, address);
POP_ENTITY_GETTER(vbk_pop_tx, pop_vbk_block_t*, published_block);
POP_ENTITY_GETTER(vbk_pop_tx, pop_array_u8_t, btc_tx);
POP_ENTITY_GETTER(vbk_pop_tx, pop_btc_block_t, block_of_proof);
POP_ENTITY_GETTER(vbk_pop_tx, pop_array_btc_block_t, block_of_proof_context);
POP_ENTITY_GETTER(vbk_pop_tx, pop_array_u8_t, signature);
POP_ENTITY_GETTER(vbk_pop_tx, pop_array_u8_t, public_key);
POP_ENTITY_GETTER(vbk_pop_tx, pop_array_u8_t, hash);
POP_ENTITY_GETTER(vbk_pop_tx, pop_merkle_path_t*, merkle_path);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_VBK_POP_TX_H
