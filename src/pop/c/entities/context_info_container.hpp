// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_CONTEXT_INFO_CONTAINER_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_CONTEXT_INFO_CONTAINER_HPP

#include "default_value.hpp"
#include "veriblock/pop/c/entities/context_info_container.h"
#include "veriblock/pop/entities/context_info_container.hpp"

struct __pop_context_info_container {
  altintegration::ContextInfoContainer ref;
};

namespace default_value {
template <>
altintegration::ContextInfoContainer
generateDefaultValue<altintegration::ContextInfoContainer>();
}

#endif
