// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_COIN_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_COIN_HPP

#include "default_value.hpp"
#include "veriblock/pop/c/entities/coin.h"
#include "veriblock/pop/entities/coin.hpp"

struct __pop_coin {
  altintegration::Coin ref;
};

namespace default_value {
template <>
altintegration::Coin generateDefaultValue<altintegration::Coin>();
}

#endif
