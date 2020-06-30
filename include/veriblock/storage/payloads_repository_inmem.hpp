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
struct PayloadsRepositoryInmem;

template <typename Payloads>
struct PayloadsWriteBatchInmem : public PayloadsWriteBatch<Payloads> {
  enum class Operation { PUT, REMOVE };

  PayloadsWriteBatchInmem(PayloadsRepositoryInmem<Payloads>* repo)
      : _repo(repo) {}

  void put(const Payloads& payloads) override {
    _ops.push_back(Operation::PUT);
    _puts.push_back(payloads);
  };

  void remove(const typename Payloads::id_t& pid) override {
    _ops.push_back(Operation::REMOVE);
    _removes.push_back(pid);
  };

  void clear() override {
    _ops.clear();
    _removes.clear();
    _puts.clear();
  };

  void commit() override {
    auto puts_begin = this->_puts.begin();
    auto removes_begin = this->_removes.begin();
    for (const auto& op : this->_ops) {
      switch (op) {
        case PayloadsWriteBatchInmem<Payloads>::Operation::PUT: {
          _repo->put(*puts_begin++);
          break;
        }
        case PayloadsWriteBatchInmem<Payloads>::Operation::REMOVE: {
          _repo->remove(*removes_begin++);
          break;
        }
        default:
          throw std::logic_error(
              "unknown enum value - this should never happen");
      }
    }
    clear();
  }

 private:
  PayloadsRepositoryInmem<Payloads>* _repo;
  std::vector<Payloads> _puts;
  std::vector<typename Payloads::id_t> _removes;
  std::vector<Operation> _ops;
};

template <typename Payloads>
struct PayloadsRepositoryInmem : public PayloadsRepository<Payloads> {
  using payloads_t = Payloads;
  using eid_t = typename Payloads::id_t;
  using cursor_t = Cursor<eid_t, Payloads>;

  ~PayloadsRepositoryInmem() override = default;

  bool remove(const eid_t& id) override { return _p.erase(id) == 1; }

  bool put(const payloads_t& payload) override {
    auto id = payload.getId();
    _p[id] = payload;
    return true;
  }

  bool get(const eid_t& id, payloads_t* payload) const override {
    auto it = _p.find(id);
    if (it == _p.end()) {
      return false;
    }
    if (payload != nullptr) {
      *payload = _p.at(id);
    }
    return true;
  }

  void clear() override { _p.clear(); }

  std::unique_ptr<PayloadsWriteBatch<Payloads>> newBatch() override {
    return std::unique_ptr<PayloadsWriteBatchInmem<Payloads>>(
        new PayloadsWriteBatchInmem<Payloads>(this));
  }

  std::shared_ptr<cursor_t> newCursor() const override {
    return std::make_shared<PayloadsCursorInmem<Payloads>>(_p);
  }

 private:
  // [endorsement id] => payload
  std::unordered_map<eid_t, payloads_t> _p;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_INMEM_HPP_
