// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

# include "context_info_container.hpp"

POP_ENTITY_FREE_SIGNATURE(context_info_container) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_GENERATE_DEFAULT_VALUE(context_info_container) {
  auto* v = new POP_ENTITY_NAME(context_info_container);
  v->ref = default_value::generateDefaultValue<altintegration::ContextInfoContainer>();
  return v;
}

namespace default_value {
template <>
altintegration::ContextInfoContainer generateDefaultValue<altintegration::ContextInfoContainer>() {
  altintegration::ContextInfoContainer res;
  res.height = 1;
  return res;
}
}  // namespace default_value
