// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/command_group_cache.hpp>
#include <utility>

#include "veriblock/pop/blockchain/command_group.hpp"
#include "veriblock/pop/hashers.hpp"

namespace altintegration {

bool CommandGroupCache::put(const CommandGroupCache::id_t& cid,
                            const std::vector<CommandGroup>& cg) {
  auto pair = _cache.insert({cid, cg});
  truncate();
  return pair.second;
}

CommandGroupCache::CommandGroupCache(const size_t maxsize)
    : _maxsize(maxsize) {}

bool CommandGroupCache::get(const CommandGroupCache::id_t& cid,
                            std::vector<CommandGroup>* out) {
  auto it = _cache.find(cid);
  if (it == _cache.end()) {
    return false;
  }

  refer(cid);
  if (out != nullptr) {
    *out = it->second;
  }

  return true;
}

bool CommandGroupCache::remove(const CommandGroupCache::id_t& cid) {
  bool res = _keys.find(cid) != _keys.end();
  _cache.erase(cid);
  if (res) {
    _priority.erase(_keys[cid]);
    _keys.erase(cid);
  }
  return res;
}

void CommandGroupCache::clear() {
  _cache.clear();
  _keys.clear();
  _priority.clear();
}

void CommandGroupCache::truncate() {
  if ((_priority.size() < _maxsize) && (_cache.size() < _maxsize)) return;

  // cache size is over the limit - erase the oldest element
  if (_priority.empty()) {
    _cache.erase(_cache.begin());
  } else {
    auto lastid = _priority.back();
    _priority.pop_back();
    _keys.erase(lastid);
    _cache.erase(lastid);
  }
}

bool CommandGroupCache::refer(const CommandGroupCache::id_t& cid) {
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

}  // namespace altintegration