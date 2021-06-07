// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_POP_CONTEXT_HPP
#define VERIBLOCK_POP_CPP_C_POP_CONTEXT_HPP

#include <memory>

#include "veriblock/pop/c/pop_context2.h"
#include "veriblock/pop/pop_context.hpp"
#include "veriblock/pop/storage/adaptors/storage_interface.hpp"
#include "veriblock/pop/storage/block_reader.hpp"

struct __pop_pop_context {
  std::shared_ptr<altintegration::PopContext> ref;
  std::shared_ptr<altintegration::adaptors::Storage> storage;
};

#endif