// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_ADDRESS_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_ADDRESS_HPP

#include "default_value.hpp"
#include "veriblock/pop/c/entities/address.h"
#include "veriblock/pop/entities/address.hpp"

struct __pop_address {
  altintegration::Address ref;
};

namespace default_value {
template <>
altintegration::Address generateDefaultValue<altintegration::Address>();
}

#endif
