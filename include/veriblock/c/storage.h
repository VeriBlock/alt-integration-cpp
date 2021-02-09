// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_H
#define VERIBLOCK_POP_CPP_STORAGE_H

#include <stdbool.h>
#include <stdint.h>

#include "veriblock/c/validation_state.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Storage Storage_t;

//! allocate new storage
Storage_t* VBK_NewStorage(const char* path, VbkValidationState* state);
//! deallocate config
void VBK_FreeStorage(Storage_t* Storage);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif