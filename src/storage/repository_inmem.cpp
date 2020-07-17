// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/storage/inmem/repository_inmem.hpp>

namespace altintegration {

bool RepositoryInmem::put(const Repository::key_t& key,
                          const Repository::value_t& value) {
  storage_[key] = value;
  // success
  return true;
}

bool RepositoryInmem::get(const Repository::key_t& key,
                          Repository::value_t* value) const {
  auto it = storage_.find(key);
  if (it == storage_.end()) {
    return false;
  }

  if (value) {
    *value = it->second;
  }

  return true;
}

std::unique_ptr<RepositoryInmem::batch_t> RepositoryInmem::newBatch() {
  return std::unique_ptr<batch_t>(new BatchInmem(this));
}

std::shared_ptr<RepositoryInmem::cursor_t> RepositoryInmem::newCursor() const {
  return std::make_shared<CursorInmem>(storage_);
}

bool RepositoryInmem::remove(const Repository::key_t& id) {
  storage_.erase(id);
  return true;  // success
}
}  // namespace altintegration