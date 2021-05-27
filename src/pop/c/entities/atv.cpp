// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>

#include <memory>

// clang-format off
#include <veriblock/pop/adaptors/picojson.hpp>
// clang-format on

#include "atv.hpp"
#include "vbkblock.hpp"
#include "veriblock/pop/assert.hpp"
#include "../validation_state2.hpp"
#include "veriblock/pop/serde.hpp"

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

POP_ENTITY_GETTER_FUNCTION(atv, POP_ARRAY_NAME(u8), id) {
  VBK_ASSERT(self);

  auto id = self->ref.getId();

  POP_ARRAY_NAME(u8) res;
  res.size = id.size();
  res.data = new uint8_t[res.size];
  std::copy(id.begin(), id.end(), res.data);

  return res;
}

POP_ENTITY_GETTER_FUNCTION(atv,
                           const POP_ENTITY_NAME(vbk_block) *,
                           block_of_proof) {
  VBK_ASSERT(self);

  auto* res = new POP_ENTITY_NAME(vbk_block);
  res->ref = self->ref.blockOfProof;

  return res;
}

POP_ENTITY_TO_JSON(atv) {
  VBK_ASSERT(self);

  std::string json =
      altintegration::ToJSON<picojson::value>(self->ref).serialize(true);

  POP_ARRAY_NAME(string) res;
  res.size = json.size();
  res.data = new char[res.size];
  strncpy(res.data, json.c_str(), res.size);

  return res;
}

POP_ENTITY_SERIALIZE_TO_VBK(atv) {
  VBK_ASSERT(self);

  auto bytes = altintegration::SerializeToVbkEncoding(self->ref);

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[bytes.size()];
  std::copy(bytes.begin(), bytes.end(), res.data);
  res.size = bytes.size();

  return res;
}

POP_ENTITY_DESERIALIZE_FROM_VBK(atv) {
  VBK_ASSERT(state);
  VBK_ASSERT(bytes.data);

  std::vector<uint8_t> v_bytes(bytes.data, bytes.data + bytes.size);

  altintegration::ATV out;
  if (!altintegration::DeserializeFromVbkEncoding(v_bytes, out, state->ref)) {
    return nullptr;
  }

  auto* res = new POP_ENTITY_NAME(atv);
  res->ref = std::move(out);
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
