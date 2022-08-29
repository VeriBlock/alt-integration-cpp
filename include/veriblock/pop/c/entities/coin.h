// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_COIN_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_COIN_H

#include "stdint.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(coin);

POP_ENTITY_GETTER_FUNCTION(coin, int64_t, units);

POP_GENERATE_DEFAULT_VALUE(coin);

#ifdef __cplusplus
}
#endif

#endif
