// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>

#include <memory>

// clang-format off
#include <veriblock/pop/adaptors/picojson.hpp>
// clang-format on

#include "altblock.hpp"
#include "veriblock/pop/assert.hpp"

POP_ENTITY_FREE_SIGNATURE(alt_block) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_GETTER_FUNCTION(alt_block, POP_ARRAY_NAME(u8), hash) {
  VBK_ASSERT(self);

  auto hash = self->ref.getHash();

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[hash.size()];
  std::copy(hash.begin(), hash.end(), res.data);
  res.size = hash.size();

  return res;
}

POP_ENTITY_GETTER_FUNCTION(alt_block, POP_ARRAY_NAME(u8), previous_block) {
  VBK_ASSERT(self);

  auto hash = self->ref.getPreviousBlock();

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[hash.size()];
  std::copy(hash.begin(), hash.end(), res.data);
  res.size = hash.size();

  return res;
}

POP_ENTITY_GETTER_FUNCTION(alt_block, uint32_t, timestamp) {
  VBK_ASSERT(self);

  return self->ref.getTimestamp();
}

POP_ENTITY_GETTER_FUNCTION(alt_block, int32_t, height) {
  VBK_ASSERT(self);

  return self->ref.getHeight();
}

POP_ENTITY_TO_JSON(alt_block, bool reverseHashes = true) {
  VBK_ASSERT(self);

  std::string json =
      altintegration::ToJSON<picojson::value>(self->ref, reverseHashes)
          .serialize(true);

  POP_ARRAY_NAME(string) res;
  res.size = json.size();
  res.data = new char[res.size];
  strncpy(res.data, json.c_str(), res.size);

  return res;
}

POP_GENERATE_DEFAULT_VALUE(alt_block) {
  auto* v = new POP_ENTITY_NAME(alt_block);
  v->ref = default_value::generateDefaultValue<altintegration::AltBlock>();
  return v;
}

namespace default_value {
template <>
altintegration::AltBlock generateDefaultValue<altintegration::AltBlock>() {
  altintegration::AltBlock res;
  res.hash = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  res.previousBlock = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
  res.timestamp = 1;
  res.height = 1;
  return res;
}
}  // namespace default_value
