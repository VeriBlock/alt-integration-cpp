// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_SERDE_H
#define VERIBLOCK_POP_CPP_SERDE_H

#include <stdint.h>

#include "array.h"

#ifdef __cplusplus
extern "C" {
#endif

#define POP_DECLARE_SERDE_VBK(prefix, type)                                  \
  void pop_##prefix##_serialize_vbk(const type* self, pop_array_u8_t** out); \
  bool pop_##prefix##_deserialize_vbk(const pop_array_u8_t* in, type** out);

#define POP_DECLARE_SERDE_RAW(prefix, type)                                  \
  void pop_##prefix##_serialize_raw(const type* self, pop_array_u8_t** out); \
  bool pop_##prefix##_deserialize_raw(const pop_array_u8_t* in, type** out);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_SERDE_H
