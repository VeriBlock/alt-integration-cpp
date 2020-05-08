// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_BTC_ENDORSEMENTS_REPOSITORY_INMEM_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_BTC_ENDORSEMENTS_REPOSITORY_INMEM_HPP_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/endorsement.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbkpoptx.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/storage/endorsement_repository.hpp"

namespace altintegration {

template <typename Endorsement>
struct EndorsementCursorInmem
    : public Cursor<typename Endorsement::id_t, Endorsement> {
  using umap = std::unordered_map<
      typename Endorsement::endorsed_hash_t,
      std::unordered_map<typename Endorsement::id_t, Endorsement>>;

  using pair = std::pair<typename Endorsement::id_t, Endorsement>;

  EndorsementCursorInmem(const umap& map) {
    for (const auto& endorsements : map) {
      for (const auto& endorsement : endorsements.second) {
        _etl.push_back(pair(endorsement.first, endorsement.second));
      }
    }
  }

  void seekToFirst() override {
    if (_etl.empty()) {
      _it = _etl.cend();
    } else {
      _it = _etl.cbegin();
    }
  }
  void seek(const typename Endorsement::id_t& key) override {
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
  typename Endorsement::id_t key() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }

    return _it->first;
  }
  Endorsement value() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }

    return _it->second;
  }

 private:
  std::vector<pair> _etl;
  typename std::vector<pair>::const_iterator _it;
};

template <typename Endorsement>
struct EndorsementRepositoryInmem : public EndorsementRepository<Endorsement> {
  using endorsement_t = Endorsement;
  using eid_t = typename Endorsement::id_t;
  using endorsed_hash_t = typename Endorsement::endorsed_hash_t;
  using containing_hash_t = typename Endorsement::containing_hash_t;
  using container_t = typename Endorsement::container_t;
  using cursor_t = Cursor<eid_t, Endorsement>;

  ~EndorsementRepositoryInmem() override = default;

  void remove(const eid_t& id) override {
    for (auto& el : e_) {
      auto& set = el.second;
      auto end = set.end();
      auto it = set.find(id);
      if (it != end) {
        set.erase(it);
        return;
      }
    }
  }

  void remove(const container_t& container) override {
    remove(Endorsement::getId(container));
  }

  void put(const container_t& container) override {
    auto e = Endorsement::fromContainer(container);
    put(e);
  }

  void put(const endorsement_t& e) override { e_[e.endorsedHash][e.id] = e; }

  std::vector<endorsement_t> get(
      const endorsed_hash_t& endorsedBlockHash) const override {
    auto it = e_.find(endorsedBlockHash);
    if (it == e_.end()) {
      return {};
    }

    auto& m = it->second;
    std::vector<endorsement_t> ret;
    ret.reserve(m.size());

    std::transform(
        m.begin(),
        m.end(),
        std::back_inserter(ret),
        [](const std::pair<eid_t, endorsement_t>& p) { return p.second; });

    return ret;
  }

  std::unique_ptr<EndorsementWriteBatch<Endorsement>> newBatch() override {
    return nullptr;
  }

  std::shared_ptr<cursor_t> newCursor() const override {
    return std::make_shared<EndorsementCursorInmem<Endorsement>>(e_);
  }

 private:
  // [endorsed block hash] => [endorsement id] => endorsement
  // clang-format off
  std::unordered_map<
    endorsed_hash_t,
    std::unordered_map<eid_t, endorsement_t>
  > e_;
  // clang-format on
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_BTC_ENDORSEMENTS_REPOSITORY_INMEM_HPP_
