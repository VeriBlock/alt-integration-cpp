// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_VBKBLOCK_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_VBKBLOCK_HPP

#include "default_value.hpp"
#include "veriblock/pop/c/entities/vbkblock.h"
#include "veriblock/pop/entities/vbkblock.hpp"

struct __pop_vbk_block {
  altintegration::VbkBlock ref;
};

namespace default_value {
template <>
altintegration::VbkBlock generateDefaultValue<altintegration::VbkBlock>();
}

#endif
