// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_TYPE_HELPERS_H
#define VERIBLOCK_POP_CPP_TYPE_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

#define POP_DECLARE_ENTITY(entity)                \
  typedef struct __pop_##entity pop_##entity##_t; \
  void pop_##entity##_free(const pop_##entity##_t* self);

// getter which allocates memory with malloc
#define POP_ENTITY_GETTER_ALLOC(entity, returntype, name) \
  void pop_##entity##_get_##name##(pop_##entity##_t * self, returntype * *out);

#define POP_ENTITY_GETTER(entity, returntype, fieldname) \
  returntype pop_##entity##_get_##fieldname(pop_##entity##_t* self);

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_TYPE_HELPERS_H
