// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <memory>

#include "vbkblock.hpp"
#include "veriblock/pop/assert.hpp"

POP_ENTITY_FREE_SIGNATURE(vbk_block) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_NEW_SIGNATURE(vbk_block) { return new POP_ENTITY_NAME(vbk_block); }

POP_ENTITY_GETTER_SIGNATURE(vbk_block, POP_ARRAY_NAME(u8), hash) {
  VBK_ASSERT(self);

  auto hash = self->ref.getHash();

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[hash.size()];
  std::copy(hash.begin(), hash.end(), res.data);
  res.size = hash.size();

  return res;
}

POP_ENTITY_GETTER_SIGNATURE(vbk_block, POP_ARRAY_NAME(u8), previous_block) {
  VBK_ASSERT(self);

  auto hash = self->ref.getPreviousBlock();

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[hash.size()];
  std::copy(hash.begin(), hash.end(), res.data);
  res.size = hash.size();

  return res;
}

POP_ENTITY_GETTER_SIGNATURE(vbk_block, POP_ARRAY_NAME(u8), merkle_root) {
  VBK_ASSERT(self);

  auto hash = self->ref.getMerkleRoot();

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[hash.size()];
  std::copy(hash.begin(), hash.end(), res.data);
  res.size = hash.size();

  return res;
}

POP_ENTITY_GETTER_SIGNATURE(vbk_block, POP_ARRAY_NAME(u8), previous_keystone) {
  VBK_ASSERT(self);

  auto hash = self->ref.getPreviousKeystone();

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[hash.size()];
  std::copy(hash.begin(), hash.end(), res.data);
  res.size = hash.size();

  return res;
}

POP_ENTITY_GETTER_SIGNATURE(vbk_block,
                            POP_ARRAY_NAME(u8),
                            second_previous_keystone) {
  VBK_ASSERT(self);

  auto hash = self->ref.getSecondPreviousKeystone();

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[hash.size()];
  std::copy(hash.begin(), hash.end(), res.data);
  res.size = hash.size();

  return res;
}

POP_ENTITY_GETTER_SIGNATURE(vbk_block, int16_t, version) {
  VBK_ASSERT(self);

  return self->ref.getVersion();
}

POP_ENTITY_GETTER_SIGNATURE(vbk_block, uint32_t, timestamp) {
  VBK_ASSERT(self);

  return self->ref.getTimestamp();
}

POP_ENTITY_GETTER_SIGNATURE(vbk_block, int32_t, difficulty) {
  VBK_ASSERT(self);

  return self->ref.getDifficulty();
}

POP_ENTITY_GETTER_SIGNATURE(vbk_block, uint64_t, nonce) {
  VBK_ASSERT(self);

  return self->ref.getNonce();
}

POP_ENTITY_GETTER_SIGNATURE(vbk_block, int32_t, height) {
  VBK_ASSERT(self);

  return self->ref.getHeight();
}

POP_GENERATE_DEFAULT_VALUE(vbk_block) {
  auto* v = new POP_ENTITY_NAME(vbk_block);

  v->ref.setNonce(1);
  v->ref.setTimestamp(1);
  v->ref.setVersion(1);
  v->ref.setDifficulty(1);
  v->ref.setHeight(1);
  v->ref.setMerkleRoot({1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
  v->ref.setPreviousBlock({1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
  v->ref.setPreviousKeystone({1, 1, 1, 1, 1, 1, 1, 1, 1});
  v->ref.setSecondPreviousKeystone({1, 1, 1, 1, 1, 1, 1, 1, 1});

  return v;
}