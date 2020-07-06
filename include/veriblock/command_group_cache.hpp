// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_COMMAND_GROUP_CACHE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_COMMAND_GROUP_CACHE_HPP_

#include <queue>
#include <veriblock/blockchain/command_group.hpp>

namespace altintegration {

static const size_t DEFAULT_CACHE_SIZE = 10000;

struct CommandGroupCache {
  using id_t = typename CommandGroup::id_t;
  CommandGroupCache(const size_t maxsize = DEFAULT_CACHE_SIZE)
      : _maxsize(maxsize) {}

  virtual ~CommandGroupCache() = default;

  virtual bool put(const CommandGroup& cg) {
    auto& id = cg.id;
    bool res = _cache.find(id) != _cache.end();
    if (!res) {
      // add new element - apply cache size limit
      if (_cache.size() >= _maxsize) {
        // cache size is over the limit - erase the oldest element
        auto& lastid = _history.front();
        _cache.erase(lastid);
        _history.pop();
      }
      _history.push(id);
    }
    _cache.insert({id, std::make_shared<CommandGroup>(cg)});
    return res;
  }

  virtual bool get(const id_t& cid, CommandGroup* out) const {
    auto it = _cache.find(cid);
    if (it == _cache.end()) {
      return false;
    }

    if (out) {
      *out = *it->second;
    }
    return true;
  }

 protected:
  size_t _maxsize;
  std::queue<id_t> _history;
  std::unordered_map<id_t, std::shared_ptr<CommandGroup>> _cache;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_COMMAND_GROUP_CACHE_HPP_
