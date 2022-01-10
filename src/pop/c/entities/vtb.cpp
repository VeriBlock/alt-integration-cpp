// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>

#include <memory>

// clang-format off
#include <veriblock/pop/adaptors/picojson.hpp>
// clang-format on

#include "vbkblock.hpp"
#include "veriblock/pop/assert.hpp"
#include "vtb.hpp"
#include "veriblock/pop/serde.hpp"
#include "../validation_state.hpp"

POP_ENTITY_FREE_SIGNATURE(vtb) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ARRAY_FREE_SIGNATURE(vtb) {
  if (self != nullptr) {
    delete[] self->data;
    self = nullptr;
  }
}

POP_ENTITY_GETTER_FUNCTION(vtb, POP_ARRAY_NAME(u8), id) {
  VBK_ASSERT(self);

  auto id = self->ref.getId();

  POP_ARRAY_NAME(u8) res;
  res.size = id.size();
  res.data = new uint8_t[res.size];
  std::copy(id.begin(), id.end(), res.data);

  return res;
}

POP_ENTITY_GETTER_FUNCTION(vtb,
                           const POP_ENTITY_NAME(vbk_block) *,
                           containing_block) {
  VBK_ASSERT(self);

  auto* res = new POP_ENTITY_NAME(vbk_block);
  res->ref = self->ref.containingBlock;

  return res;
}

POP_ENTITY_TO_JSON(vtb) {
  VBK_ASSERT(self);

  std::string json =
      altintegration::ToJSON<picojson::value>(self->ref).serialize(true);

  POP_ARRAY_NAME(string) res;
  res.size = json.size();
  res.data = new char[res.size];
  strncpy(res.data, json.c_str(), res.size);

  return res;
}

POP_ENTITY_SERIALIZE_TO_VBK(vtb) {
  VBK_ASSERT(self);

  auto bytes = altintegration::SerializeToVbkEncoding(self->ref);

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[bytes.size()];
  std::copy(bytes.begin(), bytes.end(), res.data);
  res.size = bytes.size();

  return res;
}

POP_ENTITY_DESERIALIZE_FROM_VBK(vtb) {
  VBK_ASSERT(state);
  VBK_ASSERT(bytes.data);

  std::vector<uint8_t> v_bytes(bytes.data, bytes.data + bytes.size);

  altintegration::VTB out;
  if (!altintegration::DeserializeFromVbkEncoding(v_bytes, out, state->ref)) {
    return nullptr;
  }

  auto* res = new POP_ENTITY_NAME(vtb);
  res->ref = std::move(out);
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
