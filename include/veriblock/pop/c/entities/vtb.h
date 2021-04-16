// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_VTB_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_VTB_H

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/serde.h"
#include "veriblock/pop/c/entities/vbkblock.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(vtb);

POP_ENTITY_GETTER_FUNCTION(vtb,
                          const POP_ENTITY_NAME(vbk_block) *,
                          containing_block);

POP_GENERATE_DEFAULT_VALUE(vtb);

#ifdef __cplusplus
}
#endif

#endif