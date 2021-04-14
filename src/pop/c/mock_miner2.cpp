// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "mock_miner2.hpp"

POP_ENTITY_FREE_SIGNATURE(mock_miner) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_NEW_SIGNATURE(mock_miner) { return new POP_ENTITY_NAME(mock_miner); }