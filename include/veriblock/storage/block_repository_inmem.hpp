#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_INMEM_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_INMEM_HPP_

#include <iterator>
#include <veriblock/storage/block_repository.hpp>

namespace VeriBlock {

template <typename Block>
struct CursorInmem : public Cursor<typename Block::hash_t, Block> {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;

  using umap = std::unordered_map<hash_t, std::shared_ptr<stored_block_t>>;
  using pair = std::pair<hash_t, std::shared_ptr<stored_block_t>>;

  CursorInmem(const umap& map) {
    for (const pair& m : map) {
      _etl.push_back(m);
    }
  }
  ~CursorInmem() override = default;
  void seekToFirst() override { _it = _etl.cbegin(); }
  void seek(const hash_t& key) override {
    _it = std::find_if(_etl.cbegin(), _etl.cend(), [&key](const pair& p) {
      return p.first == key;
    });
  }
  void seekToLast() override { _it = --_etl.cend(); }
  bool isValid() const override { return _it != _etl.cend(); }
  void next() override { ++_it; }
  void prev() override { _it = _it != _etl.cbegin() ? --_it : _etl.cend(); }
  hash_t key() const override { return _it->first; }
  stored_block_t value() const override { return *_it->second; }

 private:
  std::vector<pair> _etl;
  typename std::vector<pair>::const_iterator _it;
};

template <typename Block>
struct BlockRepositoryInmem;

template <typename Block>
struct WriteBatchInmem : public WriteBatch<Block> {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;

  enum class Operation { PUT, REMOVE_BY_HASH };

  void put(const stored_block_t& block) override {
    _ops.push_back(Operation::PUT);
    _puts.push_back(block);
  };

  void removeByHash(const hash_t& hash) override {
    _ops.push_back(Operation::REMOVE_BY_HASH);
    _removes.push_back(hash);
  };

  void clear() override {
    _ops.clear();
    _removes.clear();
    _puts.clear();
  };

  void commit(BlockRepository<Block>& repo) override {
    auto puts_begin = this->_puts.begin();
    auto removes_begin = this->_removes.begin();
    for (const auto& op : this->_ops) {
      switch (op) {
        case WriteBatchInmem<Block>::Operation::PUT: {
          repo.put(*puts_begin++);
          break;
        }
        case WriteBatchInmem<Block>::Operation::REMOVE_BY_HASH: {
          repo.removeByHash(*removes_begin++);
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
  std::vector<stored_block_t> _puts;
  std::vector<hash_t> _removes;
  std::vector<Operation> _ops;
};

template <typename Block>
struct BlockRepositoryInmem : public BlockRepository<Block> {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;
  //! iterator type
  using cursor_t = Cursor<hash_t, stored_block_t>;

  ~BlockRepositoryInmem() override = default;

  bool getByHash(const hash_t& hash, stored_block_t* out) const override {
    auto it = _hash.find(hash);
    if (it == _hash.end()) {
      return false;
    }

    if (out) {
      *out = *it->second;
    }
    return true;
  }

  size_t getManyByHash(Slice<const hash_t> hashes,
                       std::vector<stored_block_t>* out) const override {
    size_t totalFound = 0;
    for (const auto& h : hashes) {
      stored_block_t block;
      if (getByHash(h, &block)) {
        ++totalFound;
        out->push_back(block);
      }
    }

    return totalFound;
  }

  bool put(const stored_block_t& block) override {
    auto p = std::make_shared<stored_block_t>(block);
    auto pair = _hash.insert_or_assign(block.getHash(), std::move(p));
    return !pair.second;
  }

  bool removeByHash(const hash_t& hash) override {
    return _hash.erase(hash) == 1;
  }

  std::unique_ptr<WriteBatch<stored_block_t>> newBatch() override {
    return std::make_unique<WriteBatchInmem<stored_block_t>>();
  }

  std::shared_ptr<cursor_t> newCursor() override {
    return std::make_shared<CursorInmem<Block>>(_hash);
  }

 private:
  std::unordered_map<hash_t, std::shared_ptr<stored_block_t>> _hash;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_INMEM_HPP_
