// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_VALIDATION_STATE2_HPP
#define VERIBLOCK_POP_CPP_C_VALIDATION_STATE2_HPP

#include "veriblock/pop/c/validation_state.h"
#include "veriblock/pop/validation_state.hpp"

struct __pop_validation_state {
  altintegration::ValidationState ref;
};

#endif