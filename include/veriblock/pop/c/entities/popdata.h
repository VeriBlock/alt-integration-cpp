// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_POPDATA_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_POPDATA_H

#include <stdbool.h>

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/atv.h"
#include "veriblock/pop/c/entities/serde.h"
#include "veriblock/pop/c/entities/vbkblock.h"
#include "veriblock/pop/c/entities/vtb.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(pop_data);

POP_ENTITY_GETTER_FUNCTION(pop_data, POP_ARRAY_NAME(atv), atvs);
POP_ENTITY_GETTER_FUNCTION(pop_data, POP_ARRAY_NAME(vtb), vtbs);
POP_ENTITY_GETTER_FUNCTION(pop_data, POP_ARRAY_NAME(vbk_block), context);

POP_ENTITY_TO_JSON(pop_data, bool verbose);

POP_ENTITY_SERIALIZE_TO_VBK(pop_data);
POP_ENTITY_DESERIALIZE_FROM_VBK(pop_data);

POP_GENERATE_DEFAULT_VALUE(pop_data);

#ifdef __cplusplus
}
#endif

#endif