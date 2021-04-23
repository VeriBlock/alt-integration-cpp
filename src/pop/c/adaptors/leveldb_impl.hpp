// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ADAPTORS_STORAGE_LEVELDB_IMPL_HPP
#define VERIBLOCK_POP_CPP_ADAPTORS_STORAGE_LEVELDB_IMPL_HPP

#include "leveldb/db.h"
#include "storage_interface.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/exceptions/storage_io.hpp"
#include "veriblock/pop/strutil.hpp"

namespace adaptors {

struct LevelDBStorage : public Storage {
  ~LevelDBStorage() override;

  LevelDBStorage(const std::string& path);

  void write(const std::vector<uint8_t>& key,
             const std::vector<uint8_t>& value) override;

  bool read(const std::vector<uint8_t>& key,
            std::vector<uint8_t>& value) override;

  std::shared_ptr<WriteBatch> generateWriteBatch() override { return nullptr; }

  std::shared_ptr<StorageIterator> generateIterator() override {
    return nullptr;
  }

 private:
  leveldb::DB* db_{nullptr};
  leveldb::WriteOptions write_options_{};
  leveldb::ReadOptions read_options_{};
};

}  // namespace adaptors

#endif