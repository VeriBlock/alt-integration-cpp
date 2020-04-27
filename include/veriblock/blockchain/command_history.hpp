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

  void addFrom(const CommandHistory& other) {
    for(const auto& c : other.undo_) {
      undo_.push_back(c);
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
    undo_.back()->UnExecute();
    undo_.pop_back();
  }

  bool hasUndo() const noexcept { return !undo_.empty(); }

  void clear() { undo_.clear(); }

  std::string toPrettyString(size_t level = 0) const {
    std::ostringstream ss;
    auto pad = std::string(level, ' ');
    for (size_t i = 0, size = undo_.size(); i < size; i++) {
      const auto & cmd = undo_[i];
      ss << cmd->toPrettyString(level + 2);
      ss << ((i < size - 1) ? "\n" : "}");
    }
    return ss.str();
  }

 private:
  storage_t undo_;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_COMMAND_HISTORY_HPP
