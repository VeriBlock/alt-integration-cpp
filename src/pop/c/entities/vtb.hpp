// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_VTB_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_VTB_HPP

#include "default_value.hpp"
#include "veriblock/pop/c/entities/vtb.h"
#include "veriblock/pop/entities/vtb.hpp"

struct __pop_vtb {
  altintegration::VTB ref;
};

namespace default_value {
template <>
altintegration::VTB generateDefaultValue<altintegration::VTB>();
}

#endif
