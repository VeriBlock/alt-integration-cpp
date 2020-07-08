// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_COMMAND_GROUP_CACHE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_COMMAND_GROUP_CACHE_HPP_

#include <list>
// important to include this since it contains hasher for the vector as a key
#include <veriblock/entities/altblock.hpp>
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
    _cache.insert({id, std::make_shared<CommandGroup>(cg)});
    truncate();
    return res;
  }

  virtual bool get(const id_t& cid, CommandGroup* out) {
    auto it = _cache.find(cid);
    if (it == _cache.end()) {
      return false;
    } else {
      refer(cid);
      if (out) {
        *out = *it->second;
      }
    }
    return true;
  }

  virtual bool remove(const id_t& cid) {
    bool res = _keys.find(cid) != _keys.end();
    _cache.erase(cid);
    if (res) {
      _priority.erase(_keys[cid]);
      _keys.erase(cid);
    }
    return res;
  }

  virtual void clear() {
    _cache.clear();
    _keys.clear();
    _priority.clear();
  }

 protected:
  size_t _maxsize;
  std::list<id_t> _priority;
  std::unordered_map<id_t, std::list<id_t>::iterator> _keys;
  std::unordered_map<id_t, std::shared_ptr<CommandGroup>> _cache;

  virtual void truncate() {
    if ((_priority.size() < _maxsize) && (_cache.size() < _maxsize)) return;

    // cache size is over the limit - erase the oldest element
    auto lastid = _priority.back();
    _priority.pop_back();
    _keys.erase(lastid);
    _cache.erase(lastid);
  }

  virtual bool refer(const id_t& cid) {
    bool res = _keys.find(cid) != _keys.end();
    if (res) {
      _priority.erase(_keys[cid]);
    } else {
      truncate();
    }
    _priority.push_front(cid);
    _keys[cid] = _priority.begin();
    return res;
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_COMMAND_GROUP_CACHE_HPP_
