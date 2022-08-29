// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_ALTBLOCK_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_ALTBLOCK_HPP

#include "default_value.hpp"
#include "veriblock/pop/c/entities/altblock.h"
#include "veriblock/pop/entities/altblock.hpp"

struct __pop_alt_block {
  altintegration::AltBlock ref;
};

namespace default_value {
template <>
altintegration::AltBlock generateDefaultValue<altintegration::AltBlock>();
}

#endif
