// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "network_byte_pair.hpp"
#include "veriblock/pop/assert.hpp"

POP_ENTITY_FREE_SIGNATURE(network_byte_pair) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_GETTER_FUNCTION(network_byte_pair, bool, has_network_byte) {
  VBK_ASSERT(self);

  return self->ref.networkType.hasValue;
}

POP_ENTITY_GETTER_FUNCTION(network_byte_pair, uint8_t, network_byte) {
  VBK_ASSERT(self);

  return self->ref.networkType.value;
}

POP_ENTITY_GETTER_FUNCTION(network_byte_pair, uint8_t, type_id) {
  VBK_ASSERT(self);

  return self->ref.typeId;
}

POP_GENERATE_DEFAULT_VALUE(network_byte_pair) {
  auto* v = new POP_ENTITY_NAME(network_byte_pair);
  v->ref =
      default_value::generateDefaultValue<altintegration::NetworkBytePair>();
  return v;
}

namespace default_value {
template <>
altintegration::NetworkBytePair
generateDefaultValue<altintegration::NetworkBytePair>() {
  altintegration::NetworkBytePair res;
  res.typeId = 1;
  res.networkType.value = 1;
  res.networkType.hasValue = true;
  return res;
}
}  // namespace default_value