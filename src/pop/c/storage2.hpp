// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_STORAGE_HPP
#define VERIBLOCK_POP_CPP_C_STORAGE_HPP

#include <memory>

#include "veriblock/pop/c/storage2.h"
#include "veriblock/pop/storage/adaptors/storage_interface.hpp"

struct __pop_storage {
  std::shared_ptr<altintegration::adaptors::Storage> ref;
};

#endif