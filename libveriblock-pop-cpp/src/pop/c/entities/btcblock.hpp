// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_BTCBLOCK_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_BTCBLOCK_HPP

#include "default_value.hpp"
#include "veriblock/pop/c/entities/btcblock.h"
#include "veriblock/pop/entities/btcblock.hpp"

struct __pop_btc_block {
  altintegration::BtcBlock ref;
};

namespace default_value {
template <>
altintegration::BtcBlock generateDefaultValue<altintegration::BtcBlock>();
}

#endif