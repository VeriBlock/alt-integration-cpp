// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_CONFIG_HPP
#define VERIBLOCK_POP_CPP_C_CONFIG_HPP

#include <memory>

#include "veriblock/pop/c/config.h"
#include "veriblock/pop/config.hpp"

struct __pop_config {
  std::shared_ptr<altintegration::Config> ref;
};

#endif