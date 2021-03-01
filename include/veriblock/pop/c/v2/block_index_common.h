// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BLOCK_INDEX_COMMON___H
#define VERIBLOCK_POP_CPP_BLOCK_INDEX_COMMON___H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "serde.h"

#define POP_DECLARE_BLOCK_INDEX_COMMON(prefix, C_indextype, C_blocktype)  \
  void pop_##prefix##_block_index_free(const C_indextype* self);          \
  const C_blocktype* pop_##prefix##_block_index_get_block(                \
      const C_indextype* self);                                           \
  int32_t pop_##prefix##_block_index_get_height(const C_indextype* self); \
  const pop_array_u8_t* pop_##prefix##_block_index_get_hash(              \
      const C_indextype* self);                                           \
  uint32_t pop_##prefix##_block_index_get_status(const C_indextype* self);
#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_BLOCK_INDEX_COMMON___H
