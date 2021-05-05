// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_NETWORK_BYTE_PAIR_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_NETWORK_BYTE_PAIR_HPP

#include "default_value.hpp"
#include "veriblock/pop/c/entities/network_byte_pair.h"
#include "veriblock/pop/entities/network_byte_pair.hpp"

struct __pop_network_byte_pair {
  altintegration::NetworkBytePair ref;
};

namespace default_value {
template <>
altintegration::NetworkBytePair generateDefaultValue<altintegration::NetworkBytePair>();
}

#endif