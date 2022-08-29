// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_POP_PAYOUTS_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_POP_PAYOUTS_HPP

#include <cstdint>
#include <vector>

#include "veriblock/pop/c/entities/pop_payouts.h"
#include "veriblock/pop/entities/pop_payouts.hpp"

struct __pop_pop_payout {
  std::vector<uint8_t> payout_info;
  uint64_t amount;
};

#endif
