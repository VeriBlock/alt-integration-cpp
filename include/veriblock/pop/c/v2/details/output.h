// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_OUTPUT_H
#define VERIBLOCK_POP_CPP_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "serde.h"
#include "type_helpers.h"

POP_DECLARE_ENTITY(output);

POP_ENTITY_GETTER(output, const char*, address);
POP_ENTITY_GETTER(output, int64_t, coin);

POP_DECLARE_SERDE_VBK(output);

POP_DECLARE_ARRAY(pop_output_t, output);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_OUTPUT_H
