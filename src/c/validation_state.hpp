// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VALIDATION_STATE_HPP_BINDINGS
#define VERIBLOCK_POP_CPP_VALIDATION_STATE_HPP_BINDINGS

#include <memory>

#include "veriblock/validation_state.hpp"

struct VbkValidationState {
  altintegration::ValidationState validation_state{};
};

#endif