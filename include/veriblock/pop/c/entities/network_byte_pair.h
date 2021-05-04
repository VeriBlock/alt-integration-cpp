// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_NETWORK_BYTE_PAIR_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_NETWORK_BYTE_PAIR_H

#include "stdbool.h"
#include "stdint.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(network_byte_pair);

POP_ENTITY_GETTER_FUNCTION(network_byte_pair, bool, has_network_byte);
POP_ENTITY_GETTER_FUNCTION(network_byte_pair, uint8_t, network_byte);
POP_ENTITY_GETTER_FUNCTION(network_byte_pair, uint8_t, type_id);

POP_GENERATE_DEFAULT_VALUE(network_byte_pair);

#ifdef __cplusplus
}
#endif

#endif