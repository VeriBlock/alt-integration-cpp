// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_TX_H
#define VERIBLOCK_POP_CPP_VBK_TX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "output.h"
#include "publication_data.h"
#include "serde.h"
#include "type_helpers.h"

POP_DECLARE_ENTITY(vbk_tx);

POP_ENTITY_GETTER(vbk_tx, bool, is_mainnet);
POP_ENTITY_GETTER(vbk_tx, uint8_t, network_byte);
POP_ENTITY_GETTER(vbk_tx, const char*, source_address);
POP_ENTITY_GETTER(vbk_tx, int64_t, source_amount);
POP_ENTITY_GETTER(vbk_tx, pop_array_output_t, outputs);
POP_ENTITY_GETTER(vbk_tx, int64_t, signature_index);
POP_ENTITY_GETTER(vbk_tx, pop_publication_data_t*, publication_data);
POP_ENTITY_GETTER(vbk_tx, pop_array_u8_t, signature);
POP_ENTITY_GETTER(vbk_tx, pop_array_u8_t, public_key);
POP_ENTITY_GETTER(vbk_tx, pop_array_u8_t, hash);

POP_DECLARE_SERDE_VBK(vbk_tx);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_VBK_TX_H
