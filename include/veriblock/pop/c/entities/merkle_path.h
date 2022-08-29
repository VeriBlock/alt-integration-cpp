// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_MERKLE_PATH_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_MERKLE_PATH_H

#include "stdint.h"
#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(merkle_path);

POP_ENTITY_GETTER_FUNCTION(merkle_path, int32_t, index);
POP_ENTITY_GETTER_FUNCTION(merkle_path, POP_ARRAY_NAME(u8), subject);
POP_ENTITY_GETTER_FUNCTION(merkle_path, POP_ARRAY_NAME(array_u8), layers);

POP_GENERATE_DEFAULT_VALUE(merkle_path);

#ifdef __cplusplus
}
#endif

#endif
