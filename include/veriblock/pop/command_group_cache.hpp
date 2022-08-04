// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_COMMAND_GROUP_CACHE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_COMMAND_GROUP_CACHE_HPP_

#include <cstddef>
#include <cstdint>
#include <list>
#include <unordered_map>
#include <vector>

#include "blockchain/command_group.hpp"
#include "hashers.hpp"

namespace altintegration {
struct CommandGroup;

//! @private
struct CommandGroupCache {
  // id = containing block hash
  using id_t = std::vector<uint8_t>;
  CommandGroupCache(const size_t maxsize = 1000);

  bool put(const id_t& cid, const std::vector<CommandGroup>& cg);

  bool get(const id_t& cid, std::vector<CommandGroup>* out);

  bool remove(const id_t& cid);

  void clear();

 protected:
  size_t _maxsize;
  std::list<id_t> _priority;
  std::unordered_map<id_t, std::list<id_t>::iterator> _keys;
  // cache stores [containing block hash] => list of command groups
  std::unordered_map<id_t, std::vector<CommandGroup>> _cache;

  void truncate();

  bool refer(const id_t& cid);
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_COMMAND_GROUP_CACHE_HPP_
