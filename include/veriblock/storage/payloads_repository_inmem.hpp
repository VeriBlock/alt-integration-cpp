// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_INMEM_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_INMEM_HPP_

#include <unordered_map>

#include "veriblock/storage/payloads_repository.hpp"

namespace altintegration {

template <typename Payloads>
struct PayloadsCursorInmem : public Cursor<typename Payloads::id_t, Payloads> {
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! payloads id type
  using payloads_id = typename Payloads::id_t;

  using umap = std::unordered_map<payloads_id, stored_payloads_t>;
  using pair = std::pair<payloads_id, stored_payloads_t>;

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
  void seek(const payloads_id& key) override {
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
  payloads_id key() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }

    return _it->first;
  }
  stored_payloads_t value() const override {
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
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! payloads id type
  using payloads_id = typename Payloads::id_t;

  using pair = std::pair<payloads_id, stored_payloads_t>;

  enum class Operation { PUT, REMOVE_BY_HASH };

  ~PayloadsWriteBatchInmem() override = default;

  PayloadsWriteBatchInmem(PayloadsRepositoryInmem<stored_payloads_t>* repo)
      : _repo(repo) {}

  void put(const stored_payloads_t& payloads) override {
    _ops.push_back(Operation::PUT);
    _puts.push_back(pair(payloads.getId(), payloads));
  }

  void removeByHash(const payloads_id& id) override {
    _ops.push_back(Operation::REMOVE_BY_HASH);
    _removes.push_back(id);
  }

  void clear() override {
    _ops.clear();
    _removes.clear();
    _puts.clear();
  }

  void commit() override {
    auto puts_begin = this->_puts.begin();
    auto removes_begin = this->_removes.begin();
    for (const auto& op : this->_ops) {
      switch (op) {
        case PayloadsWriteBatchInmem<stored_payloads_t>::Operation::PUT: {
          _repo->put(puts_begin->second);
          ++puts_begin;
          break;
        }
        case PayloadsWriteBatchInmem<
            stored_payloads_t>::Operation::REMOVE_BY_HASH: {
          _repo->removeByHash(*removes_begin++);
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
  PayloadsRepositoryInmem<stored_payloads_t>* _repo;
  std::vector<pair> _puts;
  std::vector<payloads_id> _removes;
  std::vector<Operation> _ops;
};

template <typename Payloads>
struct PayloadsRepositoryInmem : public PayloadsRepository<Payloads> {
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! payloads id type
  using payloads_id = typename Payloads::id_t;
  //! iterator type
  using cursor_t = Cursor<payloads_id, stored_payloads_t>;

  ~PayloadsRepositoryInmem() override = default;

  void put(const stored_payloads_t& payloads) override {
    payloads_rep_[payloads.getId()] = payloads;
  }

  bool get(const payloads_id& id, stored_payloads_t* out) const override {
    auto it = payloads_rep_.find(id);
    if (it == payloads_rep_.end()) {
      return {};
    }

    if (out != nullptr) {
      *out = it->second;
    }

    return true;
  }

  size_t get(const std::vector<payloads_id>& ids,
             std::vector<stored_payloads_t>* out) const override {
    size_t totalFound = 0;
    for (const auto& id : ids) {
      stored_payloads_t payloads;
      if (get(id, &payloads)) {
        ++totalFound;
        out->push_back(payloads);
      }
    }

    return totalFound;
  }

  void removeByHash(const payloads_id& id) override { payloads_rep_.erase(id); }

  void clear() override { payloads_rep_.clear(); }

  std::unique_ptr<PayloadsWriteBatch<stored_payloads_t>> newBatch() override {
    return std::unique_ptr<PayloadsWriteBatchInmem<stored_payloads_t>>(
        new PayloadsWriteBatchInmem<stored_payloads_t>(this));
  }

  std::shared_ptr<cursor_t> newCursor() override {
    return std::make_shared<PayloadsCursorInmem<stored_payloads_t>>(
        payloads_rep_);
  }

 private:
  // [payloads id] => [payloads]
  std::unordered_map<payloads_id, stored_payloads_t> payloads_rep_;
};

}  // namespace altintegration

#endif
