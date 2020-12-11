// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VALIDATION_STATE_HPP_BINDINGS
#define VERIBLOCK_POP_CPP_VALIDATION_STATE_HPP_BINDINGS

#include <memory>
#include <string>

#include "veriblock/validation_state.hpp"

struct VbkValidationState {
  bool IsValid() const { return validation_state.IsValid(); }

  bool IsInvalid() const { return validation_state.IsInvalid(); }

  const char* GetErrorMessage() {
    error = validation_state.toString();
    return error.c_str();
  }

 private:
  altintegration::ValidationState validation_state{};
  std::string error;
};

#endif