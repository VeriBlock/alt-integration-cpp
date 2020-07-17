// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_REPOSITORY_INMEM_HPP
#define VERIBLOCK_POP_CPP_REPOSITORY_INMEM_HPP

#include <map>
#include <veriblock/hashers.hpp>
#include <veriblock/storage/repository.hpp>

#include "batch_inmem.hpp"
#include "cursor_inmem.hpp"

namespace altintegration {

struct RepositoryInmem : public Repository {
  using batch_t = Batch;
  using cursor_t = Repository::cursor_t;

  ~RepositoryInmem() override = default;

  bool remove(const key_t& id) override;

  bool put(const key_t& key, const value_t& value) override;

  bool get(const key_t& key, value_t* value) const override;

  std::unique_ptr<batch_t> newBatch() override;

  std::shared_ptr<cursor_t> newCursor() const override;

 private:
  std::map<key_t, value_t> storage_;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_REPOSITORY_INMEM_HPP
