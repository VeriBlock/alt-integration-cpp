// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_MATH_HPP
#define VERIBLOCK_POP_CPP_MATH_HPP

#include <cstdint>

namespace altintegration {
namespace progpow {

uint32_t popcount(uint32_t v);
uint32_t clz(uint32_t v);
uint32_t mul_hi(uint32_t a, uint32_t b);

}  // namespace progpow
}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_MATH_HPP
