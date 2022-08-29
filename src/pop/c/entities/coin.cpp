// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "coin.hpp"

#include "cstdint"
#include "veriblock/pop/assert.hpp"

POP_ENTITY_FREE_SIGNATURE(coin) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_GETTER_FUNCTION(coin, int64_t, units) {
  VBK_ASSERT(self);

  return static_cast<int64_t>(self->ref.units);
}

POP_GENERATE_DEFAULT_VALUE(coin) {
  auto* v = new POP_ENTITY_NAME(coin);
  v->ref = default_value::generateDefaultValue<altintegration::Coin>();
  return v;
}

namespace default_value {
template <>
altintegration::Coin generateDefaultValue<altintegration::Coin>() {
  altintegration::Coin res(1);
  return res;
}
}  // namespace default_value
