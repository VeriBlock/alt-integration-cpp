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