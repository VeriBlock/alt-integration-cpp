// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_V2_H
#define VERIBLOCK_POP_CPP_V2_H

#ifdef __cplusplus
extern "C" {
#endif

// list all headers from 'details'

#include "details/pop_data.h"

// do not expose internal macros
#undef POP_ENTITY_GETTER
#undef POP_ENTITY_GETTER_ALLOC
#undef POP_DECLARE_ARRAY
#undef POP_DECLARE_ENTITY
#undef POP_DECLARE_SERDE_VBK
#undef POP_DECLARE_SERDE_RAW

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_V2_H
