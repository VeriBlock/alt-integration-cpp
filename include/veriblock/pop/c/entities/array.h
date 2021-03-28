
// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ENTITIES_ARRAY_H
#define VERIBLOCK_POP_CPP_ENTITIES_ARRAY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POP_DECLARE_ARRAY(type, suffix)                       \
  struct __pop_array_##suffix {                               \
    const type* data;                                         \
    size_t size;                                              \
  };                                                          \
  typedef struct __pop_array_##suffix pop_array_##suffix##_t; \
  void pop_array_##suffix##_free(const pop_array_##suffix##_t* self);

#define POP_ARRAY_NAME(suffix) pop_array_##suffix##_t

// declare arrays of simple types here
POP_DECLARE_ARRAY(uint8_t, u8);
POP_DECLARE_ARRAY(uint32_t, u32);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_ARRAY_H