// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <memory>

#include "atv.hpp"
#include "vbkblock.hpp"
#include "veriblock/pop/assert.hpp"

POP_ENTITY_FREE_SIGNATURE(atv) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ARRAY_FREE_SIGNATURE(atv) {
  if (self != nullptr) {
    delete[] self->data;
    self = nullptr;
  }
}

POP_ENTITY_GETTER_FUNCTION(atv,
                           const POP_ENTITY_NAME(vbk_block) *,
                           block_of_proof) {
  VBK_ASSERT(self);

  auto* res = new POP_ENTITY_NAME(vbk_block);
  res->ref = self->ref.blockOfProof;

  return res;
}

POP_GENERATE_DEFAULT_VALUE(atv) {
  auto* v = new POP_ENTITY_NAME(atv);
  v->ref = default_value::generateDefaultValue<altintegration::ATV>();
  return v;
}

namespace default_value {
template <>
altintegration::ATV generateDefaultValue<altintegration::ATV>() {
  altintegration::ATV res;
  res.blockOfProof = generateDefaultValue<altintegration::VbkBlock>();
  return res;
}
}  // namespace default_value
