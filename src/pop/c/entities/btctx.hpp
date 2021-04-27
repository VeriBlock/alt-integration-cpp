// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_BTCTX_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_BTCTX_HPP

#include "default_value.hpp"
#include "veriblock/pop/c/entities/btctx.h"
#include "veriblock/pop/entities/btctx.hpp"

struct __pop_btctx {
  altintegration::BtcTx ref;
};

namespace default_value {
template <>
altintegration::BtcTx generateDefaultValue<altintegration::BtcTx>();
}

#endif