// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_STORAGE_H
#define VERIBLOCK_POP_CPP_C_STORAGE_H

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/type_helpers.h"
#include "veriblock/pop/c/validation_state.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(storage);

POP_ENTITY_NEW_FUNCTION(storage,
                        POP_ARRAY_NAME(string) path,
                        POP_ENTITY_NAME(validation_state) * state);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif