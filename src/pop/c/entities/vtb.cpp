// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <memory>

#include "vbkblock.hpp"
#include "veriblock/pop/assert.hpp"
#include "vtb.hpp"

POP_ENTITY_FREE_SIGNATURE(vtb) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ARRAY_FREE_SIGNATURE(vtb) {
  if (self != nullptr) {
    for (size_t i = 0; i < self->size; ++i) {
      POP_ENTITY_FREE(vtb)(self->data[i]);
    }
    delete[] self->data;
    self = nullptr;
  }
}

POP_ENTITY_GETTER_FUNCTION(vtb,
                           const POP_ENTITY_NAME(vbk_block) *,
                           containing_block) {
  VBK_ASSERT(self);

  auto* res = new POP_ENTITY_NAME(vbk_block);
  res->ref = self->ref.containingBlock;

  return res;
}

POP_GENERATE_DEFAULT_VALUE(vtb) {
  auto* v = new POP_ENTITY_NAME(vtb);
  v->ref = default_value::generateDefaultValue<altintegration::VTB>();

  return v;
}

namespace default_value {
template <>
altintegration::VTB generateDefaultValue<altintegration::VTB>() {
  altintegration::VTB res;
  res.containingBlock = generateDefaultValue<altintegration::VbkBlock>();
  return res;
}
}  // namespace default_value
