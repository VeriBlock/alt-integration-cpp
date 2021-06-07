// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_ATV_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_ATV_HPP

#include "default_value.hpp"
#include "veriblock/pop/c/entities/atv.h"
#include "veriblock/pop/entities/atv.hpp"

struct __pop_atv {
  altintegration::ATV ref;
};

namespace default_value {
template <>
altintegration::ATV generateDefaultValue<altintegration::ATV>();
}

#endif