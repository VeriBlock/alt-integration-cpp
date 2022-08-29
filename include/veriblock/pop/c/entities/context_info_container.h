// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_CONTEXT_INFO_CONTAINER_H
#define VERIBLOCK_POP_CPP_C_ENTITIES_CONTEXT_INFO_CONTAINER_H

#include "veriblock/pop/c/entities/serde.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(context_info_container);

POP_ENTITY_GETTER_FUNCTION(context_info_container, int, height);

POP_ENTITY_GETTER_FUNCTION(context_info_container,
                           POP_ARRAY_NAME(u8),
                           first_previous_keystone);

POP_ENTITY_GETTER_FUNCTION(context_info_container,
                           POP_ARRAY_NAME(u8),
                           second_previous_keystone);

POP_ENTITY_SERIALIZE_TO_VBK(context_info_container);
POP_ENTITY_DESERIALIZE_FROM_VBK(context_info_container);

POP_GENERATE_DEFAULT_VALUE(context_info_container);

#ifdef __cplusplus
}
#endif

#endif
