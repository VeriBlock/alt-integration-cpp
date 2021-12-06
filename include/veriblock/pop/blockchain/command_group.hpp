// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_COMMANDGROUP_HPP
#define ALTINTEGRATION_COMMANDGROUP_HPP

#include <utility>
#include <vector>
#include <veriblock/pop/reversed_range.hpp>
#include <veriblock/pop/uint.hpp>

#include "command.hpp"

namespace altintegration {

//! @private
struct CommandGroup {
  using storage_t = std::vector<CommandPtr>;
  using id_t = std::vector<uint8_t>;

  CommandGroup() = default;

  CommandGroup(std::vector<uint8_t> id,
               bool valid,
               const std::string& payload_type_name)
      : payload_type_name(&payload_type_name),
        id(std::move(id)),
        valid(valid) {}

  // HACK, store the payload type name
  const std::string* payload_type_name{};

  // ATV id or VTB id or VBK block id
  std::vector<uint8_t> id;
  storage_t commands;
  bool valid{true};

  // clang-format off
  typename storage_t::iterator begin() { return commands.begin(); }
  typename storage_t::const_iterator begin() const { return commands.begin(); }
  typename storage_t::iterator end() { return commands.end(); }
  typename storage_t::const_iterator end() const { return commands.end(); }
  typename storage_t::reverse_iterator rbegin() { return commands.rbegin(); }
  typename storage_t::const_reverse_iterator rbegin() const { return commands.rbegin(); }
  typename storage_t::reverse_iterator rend() { return commands.rend(); }
  typename storage_t::const_reverse_iterator rend() const { return commands.rend(); }
  // clang-format on

  const std::string& getPayloadsTypeName() const { return *payload_type_name; }

  /**
   * Execute all commands in the group
   * @invariant atomic: executes either all or none of the commands
   * @return true if all commands succeded, false otherwise
   */
  bool execute(ValidationState& state) const {
    for (auto cmd = begin(); cmd != end(); ++cmd) {
      if (!(*cmd)->Execute(state)) {
        // one of the commands has failed, rollback
        for (auto r_cmd = std::reverse_iterator<decltype(cmd)>(cmd);
             r_cmd != rend();
             ++r_cmd) {
          (*r_cmd)->UnExecute();
        }

        return false;
      }
    }

    return true;
  }

  /**
   * UnExecute all commands in the group
   */
  void unExecute() const noexcept {
    for (auto& cmd : reverse_iterate(commands)) {
      cmd->UnExecute();
    }
  }

  bool operator==(const CommandGroup& o) const { return id == o.id; }

  bool operator==(const uint256& o) const { return id == o; }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_COMMANDGROUP_HPP
