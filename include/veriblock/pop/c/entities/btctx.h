// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_BTCTX_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_BTCTX_H

#include "veriblock/pop/c/type_helpers.h"
#include "veriblock/pop/c/array.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(btctx);

POP_ENTITY_GETTER_FUNCTION(btctx, POP_ARRAY_NAME(u8), tx);

POP_GENERATE_DEFAULT_VALUE(btctx);

#ifdef __cplusplus
}
#endif

#endif