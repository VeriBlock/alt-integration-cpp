// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_HASH_HPP__
#define VERIBLOCK_POP_CPP_HASH_HPP__

#include <veriblock/pop/c/v2/details/array.h>

#include <veriblock/pop/slice.hpp>

inline pop_array_u8_t makeArray(altintegration::Slice<const uint8_t> slice) {
  pop_array_u8_t ret;
  ret.data = slice.data();
  ret.size = slice.size();
  return ret;
}

#endif  // VERIBLOCK_POP_CPP_HASH_HPP__
