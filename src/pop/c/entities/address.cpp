// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>

// clang-format off
#include <veriblock/pop/adaptors/picojson.hpp>
// clang-format on

#include "address.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/serde.hpp"
#include "../validation_state2.hpp"

POP_ENTITY_FREE_SIGNATURE(address) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_GETTER_FUNCTION(address, uint8_t, address_type) {
  VBK_ASSERT(self);

  return static_cast<uint8_t>(self->ref.getType());
}

POP_ENTITY_GETTER_FUNCTION(address, POP_ARRAY_NAME(string), address) {
  VBK_ASSERT(self);

  std::string addr = self->ref.toString();

  POP_ARRAY_NAME(string) res;
  res.size = addr.size();
  res.data = new char[res.size];
  strncpy(res.data, addr.c_str(), res.size);

  return res;
}

POP_ENTITY_SERIALIZE_TO_VBK(address) {
  VBK_ASSERT(self);

  auto bytes = altintegration::SerializeToVbkEncoding(self->ref);

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[bytes.size()];
  std::copy(bytes.begin(), bytes.end(), res.data);
  res.size = bytes.size();

  return res;
}

POP_ENTITY_DESERIALIZE_FROM_VBK(address) {
  VBK_ASSERT(state);
  VBK_ASSERT(bytes.data);

  std::vector<uint8_t> v_bytes(bytes.data, bytes.data + bytes.size);

  altintegration::Address out;
  if (!altintegration::DeserializeFromVbkEncoding(v_bytes, out, state->ref)) {
    return nullptr;
  }

  auto* res = new POP_ENTITY_NAME(address);
  res->ref = std::move(out);
  return res;
}

POP_GENERATE_DEFAULT_VALUE(address) {
  auto* v = new POP_ENTITY_NAME(address);
  v->ref = default_value::generateDefaultValue<altintegration::Address>();
  return v;
}

namespace default_value {
template <>
altintegration::Address generateDefaultValue<altintegration::Address>() {
  altintegration::Address res;
  altintegration::ValidationState state;
  bool success = res.fromString("V111111111111111111111111G3LuZ", state);
  VBK_ASSERT_MSG(success, state.toString());
  return res;
}
}  // namespace default_value