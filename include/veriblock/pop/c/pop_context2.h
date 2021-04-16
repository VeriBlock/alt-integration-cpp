// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_POP_CONTEXT_H
#define VERIBLOCK_POP_CPP_C_POP_CONTEXT_H

#include "veriblock/pop/c/config2.h"
#include "veriblock/pop/c/storage2.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(pop_context);

POP_ENTITY_NEW_FUNCTION(pop_context,
                        const POP_ENTITY_NAME(config) * config,
                        const POP_ENTITY_NAME(storage) * storage,
                        const char* log_lvl);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif