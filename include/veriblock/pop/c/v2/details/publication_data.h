// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_PUBLICATION_DATA_H
#define VERIBLOCK_POP_CPP_PUBLICATION_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "serde.h"
#include "type_helpers.h"

POP_DECLARE_ENTITY(publication_data);

POP_ENTITY_GETTER(publication_data, int64_t, identifier);
POP_ENTITY_GETTER(publication_data, pop_array_u8_t, header);
POP_ENTITY_GETTER(publication_data, pop_array_u8_t, payout_info);
POP_ENTITY_GETTER(publication_data, pop_array_u8_t, context_info);

POP_DECLARE_SERDE_VBK(publication_data);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_PUBLICATION_DATA_H
