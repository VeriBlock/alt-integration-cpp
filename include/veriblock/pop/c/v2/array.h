// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ARRAY_H
#define VERIBLOCK_POP_CPP_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define POP_DECLARE_ARRAY(type, suffix) \
  struct pop_array_ ##suffix## _t {        \
    const type* data;                   \
    size_t size;                        \
  };                                    \
  void pop_array_ ##suffix## _free(const pop_array_ ##suffix## _t* self);

POP_DECLARE_ARRAY(uint8_t, u8);
POP_DECLARE_ARRAY(uint32_t, u32);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_ARRAY_H
