// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_SERDE_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_SERDE_H

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/validation_state.h"

#ifdef __cplusplus
extern "C" {
#endif

#define POP_ENTITY_SERIALIZE_TO_VBK(entity) \
  POP_ARRAY_NAME(u8)                        \
  pop_##entity##_serialize_to_vbk(const pop_##entity##_t* self)

#define POP_ENTITY_DESERIALIZE_FROM_VBK(entity)          \
  pop_##entity##_t* pop_##entity##_deserialize_from_vbk( \
      POP_ARRAY_NAME(u8) bytes, POP_ENTITY_NAME(validation_state) * state)

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_C_ENTITIES_SERDE_H