// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_POP_PAYOUTS_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_POP_PAYOUTS_H

#include <stdint.h>

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/serde.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(pop_payout);

POP_ENTITY_GETTER_FUNCTION(pop_payout, POP_ARRAY_NAME(u8), payout_info);
POP_ENTITY_GETTER_FUNCTION(pop_payout, uint64_t, amount);

POP_DECLARE_ARRAY(POP_ENTITY_NAME(pop_payout) *, pop_payout);

POP_GENERATE_DEFAULT_VALUE(pop_payout);

#ifdef __cplusplus
}
#endif

#endif