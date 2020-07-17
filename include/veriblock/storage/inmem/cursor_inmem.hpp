// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CURSOR_INMEM_HPP
#define VERIBLOCK_POP_CPP_CURSOR_INMEM_HPP

#include <veriblock/storage/cursor.hpp>

namespace altintegration {

struct CursorInmem : public Cursor<std::vector<uint8_t>, std::vector<uint8_t>> {
  using key_t = std::vector<uint8_t>;
  using value_t = std::vector<uint8_t>;
  using umap = std::map<key_t, value_t>;
  using pair = std::pair<key_t, value_t>;

  ~CursorInmem() override = default;

  CursorInmem(const umap& m) {
    for (pair p : m) {
      _etl.push_back(std::move(p));
    }
  }
  void seekToFirst() override {
    if (_etl.empty()) {
      _it = _etl.cend();
    } else {
      _it = _etl.cbegin();
    }
  }
  void seek(const key_t& key) override {
    _it = std::find_if(_etl.cbegin(), _etl.cend(), [&key](const pair& p) {
      return p.first == key;
    });
  }
  void seekToLast() override { _it = --_etl.cend(); }
  bool isValid() const override {
    bool a = _it != _etl.cend();
    bool b = _it >= _etl.cbegin();
    bool c = _it < _etl.cend();
    return a && b && c;
  }
  void next() override {
    if (_it < _etl.cend()) {
      ++_it;
    }
  }
  void prev() override {
    if (_it == _etl.cbegin()) {
      _it = _etl.cend();
    } else {
      --_it;
    }
  }
  key_t key() const override {
    VBK_ASSERT(isValid() && "invalid cursor");
    return _it->first;
  }
  value_t value() const override {
    VBK_ASSERT(isValid() && "invalid cursor");
    return _it->second;
  }

 private:
  std::vector<pair> _etl;
  typename std::vector<pair>::const_iterator _it;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_CURSOR_INMEM_HPP
