// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <memory>

#include "veriblock/pop/assert.hpp"
#include "vtb.hpp"

POP_ENTITY_FREE_SIGNATURE(vtb) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_NEW_SIGNATURE(vtb) { return new POP_ENTITY_NAME(vtb); }