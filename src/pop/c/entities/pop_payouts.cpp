// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>

#include <memory>

#include "../validation_state.hpp"
#include "pop_payouts.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/serde.hpp"

POP_ENTITY_FREE_SIGNATURE(pop_payout) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ARRAY_FREE_SIGNATURE(pop_payout) {
  if (self != nullptr) {
    delete[] self->data;
    self = nullptr;
  }
}

POP_ENTITY_GETTER_FUNCTION(pop_payout, POP_ARRAY_NAME(u8), payout_info) {
  VBK_ASSERT(self);

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[self->payout_info.size()];
  std::copy(self->payout_info.begin(), self->payout_info.end(), res.data);
  res.size = self->payout_info.size();

  return res;
}

POP_ENTITY_GETTER_FUNCTION(pop_payout, uint64_t, amount) {
  VBK_ASSERT(self);

  return self->amount;
}

POP_GENERATE_DEFAULT_VALUE(pop_payout) {
  auto* v = new POP_ENTITY_NAME(pop_payout);
  v->payout_info = std::vector<uint8_t>{1, 2, 3, 4};
  v->amount = 300;
  return v;
}
