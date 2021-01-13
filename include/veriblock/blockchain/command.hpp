// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_COMMAND_HPP
#define ALTINTEGRATION_COMMAND_HPP

#include <string>
#include <veriblock/uint.hpp>
#include <veriblock/validation_state.hpp>

namespace altintegration {

//! @private
struct Command {
  virtual ~Command() = default;

  //! @invariant atomic
  VBK_CHECK_RETURN virtual bool Execute(ValidationState& state) = 0;
  virtual void UnExecute() = 0;
};

using CommandPtr = std::shared_ptr<Command>;

}  // namespace altintegration

#endif  // ALTINTEGRATION_COMMAND_HPP
