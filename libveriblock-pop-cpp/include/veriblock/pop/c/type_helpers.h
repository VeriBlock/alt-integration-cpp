// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_TYPE_HELPERS_H
#define VERIBLOCK_POP_CPP_C_TYPE_HELPERS_H

#include "veriblock/pop/c/array.h"

#ifdef __cplusplus
extern "C" {
#endif

#define POP_DECLARE_ENTITY(entity)                \
  typedef struct __pop_##entity pop_##entity##_t; \
  void pop_##entity##_free(pop_##entity##_t* self);

#define POP_ENTITY_NAME(entity) pop_##entity##_t

#define POP_ENTITY_FREE_SIGNATURE(entity) \
  void pop_##entity##_free(pop_##entity##_t* self)

#define POP_ENTITY_FREE(entity) pop_##entity##_free

// signature of the new function of the pop entity. used for the declaration
// and intialization.
#define POP_ENTITY_NEW_FUNCTION(entity, ...) \
  pop_##entity##_t* pop_##entity##_new(__VA_ARGS__)

// signature of the getter method of the pop entity. used for the declaration
// and intialization.
#define POP_ENTITY_GETTER_FUNCTION(entity, fieldtype, fieldname) \
  fieldtype pop_##entity##_get_##fieldname(const pop_##entity##_t* self)

// signature of the setter method of the pop entity. used for the declaration
// and intialization.
#define POP_ENTITY_SETTER_FUNCTION(entity, fieldtype, fieldname)    \
  void pop_##entity##_set_##fieldname(const pop_##entity##_t* self, \
                                      fieldtype val)

// signature of the toJson method of the pop entity. used for the declaration
// and intialization.
#define POP_ENTITY_TO_JSON(entity, ...) \
  POP_ARRAY_NAME(string)                \
  pop_##entity##_to_json(const pop_##entity##_t* self, ##__VA_ARGS__)

// signature of the custom function of the pop entity. used for the declaration
// and intialization.
#define POP_ENTITY_CUSTOM_FUNCTION(entity, returntype, funcname, ...)   \
  returntype pop_##entity##_function_##funcname(pop_##entity##_t* self, \
                                                ##__VA_ARGS__)

// get the default value of the entity (test used only). used for the
// declaration and intialization.
#define POP_GENERATE_DEFAULT_VALUE(entity) \
  POP_ENTITY_NAME(entity) * pop_##entity##_generate_default_value()

#define POP_GENERATE_DEFAULT_VALUE_EXECUTE(entity) \
  pop_##entity##_generate_default_value();

#define POP_DECLARE_EXTERN_FUNCTION(funcname, returntype, ...) \
  returntype pop_extern_function_##funcname(__VA_ARGS__)

#define POP_EXTERN_FUNCTION_NAME(funcname) pop_extern_function_##funcname

#ifdef __cplusplus
}
#endif

#endif  // VERIBLOCK_POP_CPP_TYPE_HELPERS_H