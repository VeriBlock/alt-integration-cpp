#ifndef ALTINTEGRATION_COMMAND_HISTORY_HPP
#define ALTINTEGRATION_COMMAND_HISTORY_HPP

#include <deque>
#include <memory>
#include <sstream>
#include <veriblock/blockchain/command.hpp>

namespace altintegration {

struct CommandHistory {
  using storage_t = std::deque<CommandPtr>;

  CommandHistory() = default;
  explicit CommandHistory(const std::vector<CommandPtr>& in) {
    for (const auto& cmd : in) {
      undo_.push_back(cmd);
    }
  }

  bool exec(const CommandPtr& cmd, ValidationState& state) {
    undo_.push_back(cmd);
    return cmd->Execute(state);
  }

  void save(std::vector<CommandPtr>& out) {
    out.reserve(out.size() + undo_.size());
    for (const auto& cmd : undo_) {
      out.push_back(cmd);
    }
    undo_.clear();
  }

  void undoAll() {
    while (hasUndo()) {
      undo();
    }
  }

  //! prior to calling this function, check if `undo` queue is empty
  void undo() {
    auto cmd = undo_.back();
    cmd->UnExecute();
    undo_.pop_back();
  }

  bool hasUndo() const noexcept { return !undo_.empty(); }

  void clear() { undo_.clear(); }

  std::string toPrettyString() const {
    std::ostringstream ss;
    ss << "History{size=" << undo_.size() << "\n";
    for (const auto& cmd : undo_) {
      ss << cmd->toPrettyString() << "\n";
    }
    ss << "}\n";
    return ss.str();
  }

 private:
  storage_t undo_;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_COMMAND_HISTORY_HPP
