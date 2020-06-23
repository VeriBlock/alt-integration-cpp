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

struct Command {
  using payload_t = bool;

  virtual ~Command() = default;

  virtual bool Execute(ValidationState& state) = 0;
  virtual void UnExecute() = 0;

  // returns unique id for this command
  virtual size_t getId() const = 0;

  //! debug method. returns a string describing this command
  virtual std::string toPrettyString(size_t level = 0) const = 0;
};

using CommandPtr = std::shared_ptr<Command>;

}  // namespace altintegration

#endif  // ALTINTEGRATION_COMMAND_HPP
