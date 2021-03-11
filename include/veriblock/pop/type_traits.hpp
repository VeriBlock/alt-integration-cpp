// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_TYPE_TRAITS_HPP
#define VERIBLOCK_POP_CPP_TYPE_TRAITS_HPP

#include <type_traits>

namespace altintegration {

template <typename T>
struct IsPopPayload {
  static const bool value = false;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_TYPE_TRAITS_HPP