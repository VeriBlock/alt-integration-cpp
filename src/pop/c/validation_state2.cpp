// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <memory>

#include "validation_state2.hpp"
#include "veriblock/pop/assert.hpp"

POP_ENTITY_FREE_SIGNATURE(validation_state) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}