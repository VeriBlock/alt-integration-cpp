// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_POPDATA_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_POPDATA_HPP

#include "default_value.hpp"
#include "veriblock/pop/c/entities/popdata.h"
#include "veriblock/pop/entities/popdata.hpp"

struct __pop_pop_data {
  altintegration::PopData ref;
};

namespace default_value {
template <>
altintegration::PopData generateDefaultValue<altintegration::PopData>();
}

#endif
