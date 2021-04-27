// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "btctx.hpp"
#include "veriblock/pop/assert.hpp"

POP_ENTITY_FREE_SIGNATURE(btctx) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_GETTER_FUNCTION(btctx, POP_ARRAY_NAME(u8), tx) {
  VBK_ASSERT(self);

  auto tx = self->ref.tx;

  POP_ARRAY_NAME(u8) res;
  res.size = tx.size();
  res.data = new uint8_t[res.size];
  std::copy(tx.begin(), tx.end(), res.data);

  return res;
}

POP_GENERATE_DEFAULT_VALUE(btctx) {
  auto* v = new POP_ENTITY_NAME(btctx);
  v->ref = default_value::generateDefaultValue<altintegration::BtcTx>();
  return v;
}

namespace default_value {
template <>
altintegration::BtcTx generateDefaultValue<altintegration::BtcTx>() {
  std::vector<uint8_t> v = {1, 1, 1, 1, 1};
  altintegration::BtcTx res(v);
  return res;
}
}  // namespace default_value
