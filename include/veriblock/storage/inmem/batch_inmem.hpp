// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BATCH_INMEM_HPP
#define VERIBLOCK_POP_CPP_BATCH_INMEM_HPP

#include <veriblock/storage/batch.hpp>

namespace altintegration {

struct Repository;

struct BatchInmem : public Batch {
  BatchInmem(Repository* repo) : _repo(repo) {}

  void put(const key_t& key, const value_t& value) override {
    _puts.push_back(std::make_pair(key, value));
  };

  void commit() override {
    for(auto& p : _puts){
      _repo->put(p.first, p.second);
    }
    _puts.clear();
  }

 private:
  Repository* _repo;
  std::vector<std::pair<key_t, value_t>> _puts;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_BATCH_INMEM_HPP
