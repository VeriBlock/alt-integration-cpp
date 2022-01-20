// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_ADDRESS_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_ADDRESS_H

#include "stdint.h"
#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/serde.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(address);

POP_ENTITY_GETTER_FUNCTION(address, uint8_t, address_type);
POP_ENTITY_GETTER_FUNCTION(address, POP_ARRAY_NAME(string), address);

POP_GENERATE_DEFAULT_VALUE(address);

#ifdef __cplusplus
}
#endif

#endif