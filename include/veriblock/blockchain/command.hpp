#ifndef ALTINTEGRATION_COMMAND_HPP
#define ALTINTEGRATION_COMMAND_HPP

#include <string>
#include <veriblock/validation_state.hpp>

namespace altintegration {

struct Command {
  virtual ~Command() = default;

  virtual bool Execute(ValidationState& state) = 0;
  virtual void UnExecute() = 0;

  //! debug method. returns a string describing this command
  virtual std::string toPrettyString() const = 0;
};

using CommandPtr = std::shared_ptr<Command>;

}  // namespace altintegration

#endif  // ALTINTEGRATION_COMMAND_HPP
