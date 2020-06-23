// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_COMMANDGROUP_HPP
#define ALTINTEGRATION_COMMANDGROUP_HPP

#include <vector>
#include <veriblock/blockchain/command.hpp>
#include <veriblock/uint.hpp>

namespace altintegration {

struct CommandGroup {
  using storage_t = std::vector<CommandPtr>;

  CommandGroup(const std::vector<uint8_t> id,
               bool valid,
               const std::string& payload_type_name)
      : payload_type_name(payload_type_name), id(id), valid(valid) {}

  // HACK, store the payload type name
  std::string payload_type_name{};

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

  std::string getPayloadsTypeName() const { return payload_type_name; }

  bool operator==(const CommandGroup& o) const { return id == o.id; }

  bool operator==(const uint256& o) const { return id == o; }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_COMMANDGROUP_HPP
