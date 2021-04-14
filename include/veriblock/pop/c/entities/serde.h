// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_SERDE_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_SERDE_H

#include <stdint.h>

#include "veriblock/pop/c/array.h"

#ifdef __cplusplus
extern "C" {
#endif

#define POP_DECLARE_SERDE_VBK(entity)                             \
  void pop_##entity##_serialize_vbk(const pop_##entity##_t* self, \
                                    pop_array_u8_t** out);        \
  bool pop_##entity##_deserialize_vbk(const pop_array_u8_t* in,   \
                                      pop_##entity##_t** out);

#define POP_DECLARE_SERDE_RAW(entity)                             \
  void pop_##entity##_serialize_raw(const pop_##entity##_t* self, \
                                    pop_array_u8_t** out);        \
  bool pop_##entity##_deserialize_raw(const pop_array_u8_t* in,   \
                                      pop_##entity##_t** out);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_SERDE_H