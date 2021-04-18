// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <memory>

#include "atv.hpp"
#include "popdata.hpp"
#include "vbkblock.hpp"
#include "veriblock/pop/assert.hpp"
#include "vtb.hpp"

POP_ENTITY_FREE_SIGNATURE(pop_data) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_GETTER_FUNCTION(pop_data, POP_ARRAY_NAME(atv), atvs) {
  VBK_ASSERT(self);

  POP_ARRAY_NAME(atv) res;
  return res;
}

POP_ENTITY_GETTER_FUNCTION(pop_data, POP_ARRAY_NAME(vtb), vtbs) {
  VBK_ASSERT(self);

  POP_ARRAY_NAME(vtb) res;
  return res;
}

POP_ENTITY_GETTER_FUNCTION(pop_data, POP_ARRAY_NAME(vbk_block), context) {
  VBK_ASSERT(self);

  POP_ARRAY_NAME(vbk_block) res;
  return res;
}