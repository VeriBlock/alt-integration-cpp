// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_ATV_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_ATV_H

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/serde.h"
#include "veriblock/pop/c/entities/vbkblock.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(atv);

POP_ENTITY_GETTER_FUNCTION(atv,
                           const POP_ENTITY_NAME(vbk_block) *,
                           block_of_proof);

POP_GENERATE_DEFAULT_VALUE(atv);

#ifdef __cplusplus
}
#endif

#endif