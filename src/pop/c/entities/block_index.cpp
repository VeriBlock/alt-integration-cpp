// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>

#include <memory>

// clang-format off
#include <veriblock/pop/adaptors/picojson.hpp>
// clang-format on

#include "block_index.hpp"
#include "veriblock/pop/assert.hpp"
#include "altblock.hpp"
#include "vbkblock.hpp"
#include "btcblock.hpp"
#include "veriblock/pop/blockchain/alt_block_tree.hpp"
#include "veriblock/pop/blockchain/pop/vbk_block_tree.hpp"

POP_ENTITY_FREE_SIGNATURE(alt_block_index) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_FREE_SIGNATURE(vbk_block_index) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_FREE_SIGNATURE(btc_block_index) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

//! get_header
POP_ENTITY_GETTER_FUNCTION(alt_block_index,
                           POP_ENTITY_NAME(alt_block) *,
                           header) {
  VBK_ASSERT(self);

  auto *res = new POP_ENTITY_NAME(alt_block);
  res->ref = self->ref.getHeader();
  return res;
}

POP_ENTITY_GETTER_FUNCTION(vbk_block_index,
                           POP_ENTITY_NAME(vbk_block) *,
                           header) {
  VBK_ASSERT(self);

  auto *res = new POP_ENTITY_NAME(vbk_block);
  res->ref = self->ref.getHeader();
  return res;
}

POP_ENTITY_GETTER_FUNCTION(btc_block_index,
                           POP_ENTITY_NAME(btc_block) *,
                           header) {
  VBK_ASSERT(self);

  auto *res = new POP_ENTITY_NAME(btc_block);
  res->ref = self->ref.getHeader();
  return res;
}

//! get_status
POP_ENTITY_GETTER_FUNCTION(alt_block_index, uint32_t, status) {
  VBK_ASSERT(self);

  return self->ref.getStatus();
}

POP_ENTITY_GETTER_FUNCTION(vbk_block_index, uint32_t, status) {
  VBK_ASSERT(self);

  return self->ref.getStatus();
}

POP_ENTITY_GETTER_FUNCTION(btc_block_index, uint32_t, status) {
  VBK_ASSERT(self);

  return self->ref.getStatus();
}

//! toJSON
POP_ENTITY_TO_JSON(alt_block_index) {
  VBK_ASSERT(self);

  std::string json =
      altintegration::ToJSON<picojson::value>(self->ref).serialize(true);

  POP_ARRAY_NAME(string) res;
  res.size = json.size();
  res.data = new char[res.size];
  strncpy(res.data, json.c_str(), res.size);

  return res;
}

POP_ENTITY_TO_JSON(vbk_block_index) {
  VBK_ASSERT(self);

  std::string json =
      altintegration::ToJSON<picojson::value>(self->ref).serialize(true);

  POP_ARRAY_NAME(string) res;
  res.size = json.size();
  res.data = new char[res.size];
  strncpy(res.data, json.c_str(), res.size);

  return res;
}

POP_ENTITY_TO_JSON(btc_block_index) {
  VBK_ASSERT(self);

  std::string json =
      altintegration::ToJSON<picojson::value>(self->ref).serialize(true);

  POP_ARRAY_NAME(string) res;
  res.size = json.size();
  res.data = new char[res.size];
  strncpy(res.data, json.c_str(), res.size);

  return res;
}