// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "merkle_path.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/uint.hpp"

POP_ENTITY_FREE_SIGNATURE(merkle_path) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_GETTER_FUNCTION(merkle_path, int32_t, index) {
  VBK_ASSERT(self);

  return self->ref.index;
}

POP_ENTITY_GETTER_FUNCTION(merkle_path, POP_ARRAY_NAME(u8), subject) {
  VBK_ASSERT(self);

  auto v = self->ref.subject.asVector();

  POP_ARRAY_NAME(u8) res;
  res.size = v.size();
  res.data = new uint8_t[res.size];
  std::copy(v.begin(), v.end(), res.data);

  return res;
}

POP_ENTITY_GETTER_FUNCTION(merkle_path, POP_ARRAY_NAME(array_u8), layers) {
  VBK_ASSERT(self);

  auto v = self->ref.layers;

  POP_ARRAY_NAME(array_u8) res;
  res.size = v.size();
  res.data = new POP_ARRAY_NAME(u8)[res.size];
  for (int i = 0; i < int(res.size); i++) {
      auto src = v[i];
      auto dst = &res.data[i];
      (*dst).size = src.size();
      (*dst).data = new uint8_t[(*dst).size];
      std::copy(src.begin(), src.end(), (*dst).data);
  }

  return res;
}

POP_GENERATE_DEFAULT_VALUE(merkle_path) {
  auto* v = new POP_ENTITY_NAME(merkle_path);
  v->ref = default_value::generateDefaultValue<altintegration::MerklePath>();
  return v;
}

namespace default_value {
template <>
altintegration::MerklePath generateDefaultValue<altintegration::MerklePath>() {
  altintegration::uint256 value{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};

  altintegration::MerklePath res;
  res.layers = std::vector<altintegration::uint256>{value, value, value};
  res.subject = value;
  res.index = 1;
  return res;
}
}  // namespace default_value