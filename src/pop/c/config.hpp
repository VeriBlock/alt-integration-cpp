// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CONFIG_HPP_BINDINGS
#define VERIBLOCK_POP_CPP_CONFIG_HPP_BINDINGS

#include <memory>

#include <veriblock/pop/config.hpp>

struct Config {
  std::shared_ptr<altintegration::Config> config;
};

#endif  // VERIBLOCK_POP_CPP_CONFIG_HPP_BINDINGS
