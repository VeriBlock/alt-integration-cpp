// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_MERKLE_PATH_H
#define VERIBLOCK_POP_CPP_MERKLE_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "serde.h"
#include "type_helpers.h"
#include "uint.h"

// used for both VbkMerklePath and MerklePath
POP_DECLARE_ENTITY(merkle_path);

POP_ENTITY_GETTER(merkle_path, int32_t, treeIndex);
POP_ENTITY_GETTER(merkle_path, int32_t, index);
POP_ENTITY_GETTER(merkle_path, pop_array_u8_t, subject);
POP_ENTITY_GETTER(merkle_path, pop_array_u256_t, layers);

POP_DECLARE_SERDE_VBK(merkle_path);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_MERKLE_PATH_H
