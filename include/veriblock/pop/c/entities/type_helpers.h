// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ENTITIES_TYPE_HELPERS_H
#define VERIBLOCK_POP_CPP_ENTITIES_TYPE_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

#define POP_DECLARE_ENTITY(entity)                \
  typedef struct __pop_##entity pop_##entity##_t; \
  void pop_##entity##_free(pop_##entity##_t* self);

#define POP_ENTITY_NAME(entity) pop_##entity##_t

#define POP_ENTITY_FREE_SIGNATURE(entity) \
  void pop_##entity##_free(pop_##entity##_t* self)

// declare the getter method for the pop entity
#define POP_ENTITY_DECLARE_GETTER(entity, returntype, fieldname) \
  returntype pop_##entity##_get_##fieldname(const pop_##entity##_t* self);

// signature of the getter method of the pop entity
#define POP_ENTITY_GETTER_SIGNATURE(entity, returntype, fieldname) \
  returntype pop_##entity##_get_##fieldname(const pop_##entity##_t* self)

// get the default value of the entity (test used only)
#define POP_GENERATE_DEFAULT_VALUE(entity) \
  POP_ENTITY_NAME(entity) * pop_##entity##_generate_default_value()

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_TYPE_HELPERS_H