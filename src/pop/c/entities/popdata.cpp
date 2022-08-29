// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>

#include <memory>

// clang-format off
#include <veriblock/pop/adaptors/picojson.hpp>
// clang-format on

#include "../validation_state.hpp"
#include "atv.hpp"
#include "popdata.hpp"
#include "vbkblock.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/serde.hpp"
#include "vtb.hpp"

POP_ENTITY_FREE_SIGNATURE(pop_data) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_GETTER_FUNCTION(pop_data, POP_ARRAY_NAME(atv), atvs) {
  VBK_ASSERT(self);

  auto& atvs = self->ref.atvs;

  POP_ARRAY_NAME(atv) res;
  res.size = atvs.size();
  res.data = new POP_ENTITY_NAME(atv)*[atvs.size()];
  for (size_t i = 0; i < res.size; ++i) {
    res.data[i] = new POP_ENTITY_NAME(atv);
    res.data[i]->ref = atvs[i];
  }
  return res;
}

POP_ENTITY_GETTER_FUNCTION(pop_data, POP_ARRAY_NAME(vtb), vtbs) {
  VBK_ASSERT(self);

  auto& vtbs = self->ref.vtbs;

  POP_ARRAY_NAME(vtb) res;
  res.size = vtbs.size();
  res.data = new POP_ENTITY_NAME(vtb)*[vtbs.size()];
  for (size_t i = 0; i < res.size; ++i) {
    res.data[i] = new POP_ENTITY_NAME(vtb);
    res.data[i]->ref = vtbs[i];
  }
  return res;
}

POP_ENTITY_GETTER_FUNCTION(pop_data, POP_ARRAY_NAME(vbk_block), context) {
  VBK_ASSERT(self);

  auto& context = self->ref.context;

  POP_ARRAY_NAME(vbk_block) res;
  res.size = context.size();
  res.data = new POP_ENTITY_NAME(vbk_block)*[context.size()];
  for (size_t i = 0; i < res.size; ++i) {
    res.data[i] = new POP_ENTITY_NAME(vbk_block);
    res.data[i]->ref = context[i];
  }
  return res;
}

POP_ENTITY_TO_JSON(pop_data, bool verbose = false) {
  VBK_ASSERT(self);

  std::string json = altintegration::ToJSON<picojson::value>(self->ref, verbose)
                         .serialize(true);

  POP_ARRAY_NAME(string) res;
  res.size = json.size();
  res.data = new char[res.size];
  strncpy(res.data, json.c_str(), res.size);

  return res;
}

POP_ENTITY_SERIALIZE_TO_VBK(pop_data) {
  VBK_ASSERT(self);

  auto bytes = altintegration::SerializeToVbkEncoding(self->ref);

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[bytes.size()];
  std::copy(bytes.begin(), bytes.end(), res.data);
  res.size = bytes.size();

  return res;
}

POP_ENTITY_DESERIALIZE_FROM_VBK(pop_data) {
  VBK_ASSERT(state);
  VBK_ASSERT(bytes.data);

  std::vector<uint8_t> v_bytes(bytes.data, bytes.data + bytes.size);

  altintegration::PopData out;
  if (!altintegration::DeserializeFromVbkEncoding(v_bytes, out, state->ref)) {
    return nullptr;
  }

  auto* res = new POP_ENTITY_NAME(pop_data);
  res->ref = std::move(out);
  return res;
}

POP_GENERATE_DEFAULT_VALUE(pop_data) {
  auto* v = new POP_ENTITY_NAME(pop_data);
  v->ref = default_value::generateDefaultValue<altintegration::PopData>();
  return v;
}

namespace default_value {
template <>
altintegration::PopData generateDefaultValue<altintegration::PopData>() {
  altintegration::PopData res;
  res.atvs = std::vector<altintegration::ATV>(
      10, generateDefaultValue<altintegration::ATV>());
  res.vtbs = std::vector<altintegration::VTB>(
      10, generateDefaultValue<altintegration::VTB>());
  res.context = std::vector<altintegration::VbkBlock>(
      10, generateDefaultValue<altintegration::VbkBlock>());

  return res;
}
}  // namespace default_value
