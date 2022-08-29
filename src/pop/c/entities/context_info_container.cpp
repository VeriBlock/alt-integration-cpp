// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "context_info_container.hpp"

#include "../validation_state.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/serde.hpp"

POP_ENTITY_FREE_SIGNATURE(context_info_container) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_GENERATE_DEFAULT_VALUE(context_info_container) {
  auto* v = new POP_ENTITY_NAME(context_info_container);
  v->ref = default_value::generateDefaultValue<
      altintegration::ContextInfoContainer>();
  return v;
}

POP_ENTITY_GETTER_FUNCTION(context_info_container, int, height) {
  VBK_ASSERT(self);
  return self->ref.height;
}

POP_ENTITY_GETTER_FUNCTION(context_info_container,
                           POP_ARRAY_NAME(u8),
                           first_previous_keystone) {
  VBK_ASSERT(self);

  auto keystone = self->ref.keystones.firstPreviousKeystone;

  POP_ARRAY_NAME(u8) res;
  res.size = keystone.size();
  res.data = new uint8_t[res.size];
  std::copy(keystone.begin(), keystone.end(), res.data);

  return res;
}

POP_ENTITY_GETTER_FUNCTION(context_info_container,
                           POP_ARRAY_NAME(u8),
                           second_previous_keystone) {
  VBK_ASSERT(self);

  auto keystone = self->ref.keystones.secondPreviousKeystone;

  POP_ARRAY_NAME(u8) res;
  res.size = keystone.size();
  res.data = new uint8_t[res.size];
  std::copy(keystone.begin(), keystone.end(), res.data);

  return res;
}

POP_ENTITY_SERIALIZE_TO_VBK(context_info_container) {
  VBK_ASSERT(self);

  auto bytes = altintegration::SerializeToVbkEncoding(self->ref);

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[bytes.size()];
  std::copy(bytes.begin(), bytes.end(), res.data);
  res.size = bytes.size();

  return res;
}

POP_ENTITY_DESERIALIZE_FROM_VBK(context_info_container) {
  VBK_ASSERT(state);
  VBK_ASSERT(bytes.data);

  std::vector<uint8_t> v_bytes(bytes.data, bytes.data + bytes.size);

  altintegration::ContextInfoContainer out;
  if (!altintegration::DeserializeFromVbkEncoding(v_bytes, out, state->ref)) {
    return nullptr;
  }

  auto* res = new POP_ENTITY_NAME(context_info_container);
  res->ref = std::move(out);
  return res;
}

namespace default_value {
template <>
altintegration::ContextInfoContainer
generateDefaultValue<altintegration::ContextInfoContainer>() {
  altintegration::ContextInfoContainer res;
  altintegration::KeystoneContainer keystones;

  keystones.firstPreviousKeystone = std::vector<uint8_t>{1};
  keystones.secondPreviousKeystone = std::vector<uint8_t>{2};

  res.keystones = keystones;
  res.height = 1;

  return res;
}
}  // namespace default_value
