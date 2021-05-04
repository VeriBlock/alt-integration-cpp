// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_PUBLICATION_DATA_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_PUBLICATION_DATA_H

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/serde.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(publication_data);

POP_ENTITY_TO_JSON(publication_data);

#ifdef __cplusplus
}
#endif

#endif