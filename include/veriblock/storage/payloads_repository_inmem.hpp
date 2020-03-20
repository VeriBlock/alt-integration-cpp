#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_INMEM_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_REPOSITORY_INMEM_HPP_

#include <unordered_map>
#include <unordered_set>

#include "veriblock/storage/payloads_repository.hpp"

namespace altintegration {

template <typename Block, typename Payloads>
struct PayloadsCursorInmem
    : public Cursor<typename Block::hash_t,
                    typename PayloadsRepository<Block, Payloads>::
                        stored_payloads_container_t> {
  //! block type
  using block_t = Block;
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! block hash type
  using hash_t = typename Block::hash_t;
  //! stored payloads container
  using stored_payloads_container_t =
      typename PayloadsRepository<Block, Payloads>::stored_payloads_container_t;

  using umap =
      std::unordered_map<hash_t, std::unordered_set<stored_payloads_t>>;
  using pair = std::pair<hash_t, std::unordered_set<stored_payloads_t>>;

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
  void seek(const hash_t& key) override {
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
  hash_t key() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }

    return _it->first;
  }
  stored_payloads_container_t value() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }

    return std::vector<stored_payloads_t>(_it->second.begin(),
                                          _it->second.end());
  }

 private:
  std::vector<pair> _etl;
  typename std::vector<pair>::const_iterator _it;
};

template <typename Block, typename Payloads>
struct PayloadsRepositoryInmem;

template <typename Block, typename Payloads>
struct PayloadsWritebatchInmem : public PayloadsWriteBatch<Block, Payloads> {
  //! block type
  using block_t = Block;
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! block hash type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;

  using pair = std::pair<hash_t, stored_payloads_t>;

  enum class Operation { PUT, REMOVE_BY_HASH };

  ~PayloadsWritebatchInmem() override = default;

  PayloadsWritebatchInmem(
      PayloadsRepositoryInmem<block_t, stored_payloads_t>* repo)
      : _repo(repo) {}

  void put(const hash_t& hash, const stored_payloads_t& payloads) override {
    _ops.push_back(Operation::PUT);
    _puts.push_back(pair(hash, payloads));
  }

  void removeByHash(const hash_t& hash) override {
    _ops.push_back(Operation::REMOVE_BY_HASH);
    _removes.push_back(hash);
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
        case PayloadsWritebatchInmem<block_t,
                                     stored_payloads_t>::Operation::PUT: {
          _repo->put(puts_begin->first, puts_begin->second);
          ++puts_begin;
          break;
        }
        case PayloadsWritebatchInmem<block_t, stored_payloads_t>::Operation::
            REMOVE_BY_HASH: {
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
  PayloadsRepositoryInmem<block_t, stored_payloads_t>* _repo;
  std::vector<pair> _puts;
  std::vector<hash_t> _removes;
  std::vector<Operation> _ops;
};

template <typename Block, typename Payloads>
struct PayloadsRepositoryInmem : public PayloadsRepository<Block, Payloads> {
  //! block type
  using block_t = Block;
  //! stored payloads type
  using stored_payloads_t = Payloads;
  //! block hash type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;
  //! stored payloads container type
  using stored_payloads_container_t = std::vector<stored_payloads_t>;
  //! iterator type
  using cursor_t = Cursor<hash_t, stored_payloads_container_t>;

  ~PayloadsRepositoryInmem() override = default;

  void put(const hash_t& hash, const stored_payloads_t& payloads) override {
    payloads_rep_[hash].insert(payloads);
  }

  stored_payloads_container_t get(const hash_t& hash) const override {
    auto it = payloads_rep_.find(hash);
    if (it == payloads_rep_.end()) {
      return {};
    }
    return std::vector<stored_payloads_t>(it->second.begin(), it->second.end());
  }

  void removeByHash(const hash_t& hash) override {
    payloads_rep_[hash].clear();
  }

  void clear() override { payloads_rep_.clear(); }

  std::unique_ptr<PayloadsWriteBatch<block_t, stored_payloads_t>> newBatch()
      override {
    return std::unique_ptr<PayloadsWritebatchInmem<block_t, stored_payloads_t>>(
        new PayloadsWritebatchInmem<block_t, stored_payloads_t>(this));
  }

  std::shared_ptr<cursor_t> newCursor() override {
    return std::make_shared<PayloadsCursorInmem<block_t, stored_payloads_t>>(
        payloads_rep_);
  }

 private:
  // [block hash] => [set payloads]
  std::unordered_map<hash_t, std::unordered_set<stored_payloads_t>>
      payloads_rep_;
};

}  // namespace altintegration

#endif
