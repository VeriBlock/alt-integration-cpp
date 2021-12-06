// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_COMMAND_HPP
#define ALTINTEGRATION_COMMAND_HPP

#include <string>
#include <veriblock/pop/uint.hpp>
#include <veriblock/pop/validation_state.hpp>

namespace altintegration {

//! @private
struct Command {
  virtual ~Command() = default;

  //! @invariant atomic
  VBK_CHECK_RETURN virtual bool Execute(ValidationState& state) noexcept = 0;
  virtual void UnExecute() noexcept = 0;
};

//! @private
using CommandPtr = std::shared_ptr<Command>;

}  // namespace altintegration

#endif  // ALTINTEGRATION_COMMAND_HPP
