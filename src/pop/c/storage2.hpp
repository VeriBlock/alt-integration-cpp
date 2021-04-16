// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_MOCK_MINER_HPP
#define VERIBLOCK_POP_CPP_C_MOCK_MINER_HPP

#include "veriblock/pop/c/storage2.h"
#include "adaptors/storage_interface.hpp"

struct __pop_storage {
  std::shared_ptr<adaptors::Storage> ref;
};

#endif