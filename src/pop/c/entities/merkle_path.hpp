// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_MERKLE_PATH_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_MERKLE_PATH_HPP

#include "default_value.hpp"
#include "veriblock/pop/c/entities/merkle_path.h"
#include "veriblock/pop/entities/merkle_path.hpp"

struct __pop_merkle_path {
  altintegration::MerklePath ref;
};

namespace default_value {
template <>
altintegration::MerklePath generateDefaultValue<altintegration::MerklePath>();
}

#endif