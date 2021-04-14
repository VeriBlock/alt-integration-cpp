// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <memory>

#include "altblock.hpp"

POP_ENTITY_FREE_SIGNATURE(alt_block) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_NEW_SIGNATURE(alt_block) { return new POP_ENTITY_NAME(alt_block); }

POP_ENTITY_GETTER_SIGNATURE(alt_block, POP_ARRAY_NAME(u8), hash) {
  auto hash = self->ref.getHash();

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[hash.size()];
  std::copy(hash.begin(), hash.end(), res.data);
  res.size = hash.size();

  return res;
}

POP_ENTITY_GETTER_SIGNATURE(alt_block, POP_ARRAY_NAME(u8), previous_block) {
  auto hash = self->ref.getPreviousBlock();

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[hash.size()];
  std::copy(hash.begin(), hash.end(), res.data);
  res.size = hash.size();

  return res;
}

POP_ENTITY_GETTER_SIGNATURE(alt_block, uint32_t, timestamp) {
  return self->ref.getTimestamp();
}

POP_ENTITY_GETTER_SIGNATURE(alt_block, int32_t, height) {
  return self->ref.getHeight();
}

POP_GENERATE_DEFAULT_VALUE(alt_block) {
  auto* v = new POP_ENTITY_NAME(alt_block);

  v->ref.hash = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  v->ref.previousBlock = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  v->ref.timestamp = 1;
  v->ref.height = 1;

  return v;
}
