#ifndef ALTINTEGRATION_COMMAND_HISTORY_HPP
#define ALTINTEGRATION_COMMAND_HISTORY_HPP

#include <memory>
#include <sstream>
#include <deque>
#include <veriblock/blockchain/command.hpp>

namespace altintegration {

struct CommandHistory {
  using storage_t = std::deque<CommandPtr>;

  bool exec(const CommandPtr& cmd, ValidationState& state) {
    undo_.push_back(cmd);
    return cmd->Execute(state);
  }

  void undoAll() {
    while (hasUndo()) {
      undo();
    }
  }

  void undo() {
    auto cmd = undo_.back();
    cmd->UnExecute();
    undo_.pop_back();
    redo_.push_back(cmd);
  }

  bool redo(ValidationState& state) {
    auto cmd = redo_.back();
    bool ret = cmd->Execute(state);
    redo_.pop_back();
    undo_.push_back(cmd);
    return ret;
  }

  bool hasUndo() const noexcept { return !undo_.empty(); }
  bool hasRedo() const noexcept { return !redo_.empty(); }

  void clear() {
    undo_.clear();
    redo_.clear();
  }

 private:
  storage_t undo_;
  storage_t redo_;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_COMMAND_HISTORY_HPP
