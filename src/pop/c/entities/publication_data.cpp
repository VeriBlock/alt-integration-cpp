// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>

#include <memory>

// clang-format off
#include <veriblock/pop/adaptors/picojson.hpp>
// clang-format on

#include "publication_data.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/serde.hpp"
#include "../validation_state2.hpp"

POP_ENTITY_FREE_SIGNATURE(publication_data) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_SERIALIZE_TO_VBK(publication_data) {
  VBK_ASSERT(self);

  auto bytes = altintegration::SerializeToVbkEncoding(self->ref);

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[bytes.size()];
  std::copy(bytes.begin(), bytes.end(), res.data);
  res.size = bytes.size();

  return res;
}

POP_ENTITY_DESERIALIZE_FROM_VBK(publication_data) {
  VBK_ASSERT(state);
  VBK_ASSERT(bytes.data);

  std::vector<uint8_t> v_bytes(bytes.data, bytes.data + bytes.size);

  altintegration::PublicationData out;
  if (!altintegration::DeserializeFromVbkEncoding(v_bytes, out, state->ref)) {
    return nullptr;
  }

  auto* res = new POP_ENTITY_NAME(publication_data);
  res->ref = std::move(out);
  return res;
}

POP_ENTITY_TO_JSON(publication_data) {
  VBK_ASSERT(self);

  std::string json =
      altintegration::ToJSON<picojson::value>(self->ref).serialize(true);

  POP_ARRAY_NAME(string) res;
  res.size = json.size();
  res.data = new char[res.size];
  strncpy(res.data, json.c_str(), res.size);

  return res;
}