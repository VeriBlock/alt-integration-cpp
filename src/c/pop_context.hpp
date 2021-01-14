// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_POP_CONTEXT_HPP_BINDINGS
#define VERIBLOCK_POP_CPP_POP_CONTEXT_HPP_BINDINGS

#include <memory>

#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/pop_context.hpp"
#include "veriblock/storage/block_provider.hpp"

// all-in-one thing
struct PopContext {
  std::shared_ptr<altintegration::PopContext> context;
  std::shared_ptr<altintegration::PayloadsProvider> payloads_provider;
  std::shared_ptr<altintegration::BlockProvider> block_provider;
};

#endif  // VERIBLOCK_POP_CPP_CONFIG_HPP_BINDINGS
