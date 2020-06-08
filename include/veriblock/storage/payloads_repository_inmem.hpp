// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_INMEM_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_INMEM_HPP_

#include <string>
#include <unordered_map>
#include "veriblock/storage/payloads_repository.hpp"

namespace altintegration {

template <typename Payloads>
struct PayloadsCursorInmem : public Cursor<typename Payloads::id_t, Payloads> {
  using umap = std::unordered_map<typename Payloads::id_t, Payloads>;
  using pair = std::pair<typename Payloads::id_t, Payloads>;

  PayloadsCursorInmem(const umap& map) {
    for (const pair& m : map) {
      _etl.push_back(m);
    }
  }

  ~PayloadsCursorInmem() override = default;

  void seekToFirst() override {
    if (_etl.empty()) {
      _it = _etl.cend();
    } else {
      _it = _etl.cbegin();
    }
  }
  void seek(const typename Payloads::id_t& key) override {
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
  typename Payloads::id_t key() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }

    return _it->first;
  }
  Payloads value() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }

    return _it->second;
  }

 private:
  std::vector<pair> _etl;
  typename std::vector<pair>::const_iterator _it;
};

template <typename Payloads>
struct PayloadsRepositoryInmem : public PayloadsRepository<Payloads> {
  using payloads_t = Payloads;
  using eid_t = typename Payloads::id_t;
  using cursor_t = Cursor<eid_t, Payloads>;

  ~PayloadsRepositoryInmem() override = default;

  bool remove(const eid_t& id) override { return p_.erase(id) == 1; }

  bool put(const payloads_t& payload) override {
    auto id = payload.getId();
    bool res = p_.find(id) != p_.end();
    p_[id] = payload;
    return res;
  }

  bool get(const eid_t& id, payloads_t* payload) const override {
    auto it = p_.find(id);
    if (it == p_.end()) {
      return false;
    }
    if (payload != nullptr) {
      *payload = p_.at(id);
    }
    return true;
  }

  std::unique_ptr<PayloadsWriteBatch<Payloads>> newBatch() override {
    return nullptr;
  }

  std::shared_ptr<cursor_t> newCursor() const override {
    return std::make_shared<PayloadsCursorInmem<Payloads>>(p_);
  }

 private:
  // [endorsement id] => payload
  std::unordered_map<eid_t, payloads_t> p_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_INMEM_HPP_
