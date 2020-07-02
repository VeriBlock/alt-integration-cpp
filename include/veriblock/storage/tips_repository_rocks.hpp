// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_TIPS_REPOSITORY_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_TIPS_REPOSITORY_ROCKS_HPP_

#include <rocksdb/db.h>

#include <set>

#include "veriblock/blob.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/storage/tips_repository.hpp"
#include "veriblock/storage/db_error.hpp"
#include "veriblock/storage/rocks_util.hpp"
#include "veriblock/strutil.hpp"

namespace altintegration {

template <typename Block>
std::string tipsName();

template <>
inline std::string tipsName<BlockIndex<BtcBlock>>() {
  return "btc_tips";
}

template <>
inline std::string tipsName<BlockIndex<VbkBlock>>() {
  return "vbk_tips";
}

template <>
inline std::string tipsName<BlockIndex<AltBlock>>() {
  return "alt_tips";
}

template <typename Block>
std::vector<uint8_t> serializeTipsToRocks(Block from) {
  WriteStream s;
  auto hash = from.getHash();
  writeSingleByteLenValue(s, hash);
  s.writeBE<uint32_t>((uint32_t)from.height);
  return s.data();
}

template <typename Block>
std::pair<typename Block::height_t, typename Block::hash_t>
deserializeTipsFromRocks(const std::string& from) {
  ReadStream stream(from);
  auto hashBytes = readSingleByteLenValue(stream).asVector();
  auto height = stream.readBE<uint32_t>();
  return std::pair<typename Block::height_t, typename Block::hash_t>{height,
                                                                     hashBytes};
}

//! column family type
using cf_handle_t = rocksdb::ColumnFamilyHandle;

template <typename Block>
class TipsRepositoryRocks : public TipsRepository<Block> {
 public:
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;

 public:
  TipsRepositoryRocks() = default;

  TipsRepositoryRocks(RepositoryRocksManager& manager,
                      const std::string& name) {
    _columnHandle = manager.getColumn(name);
    _db = manager.getDB();
  }

  bool put(const stored_block_t& tip) override {
    auto blockBytes = serializeTipsToRocks(tip);

    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s = _db->Put(write_options,
                                 _columnHandle,
                                 tipsName<stored_block_t>(),
                                 makeRocksSlice(blockBytes));
    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }
    return true;
  }

  bool get(std::pair<typename Block::height_t, typename Block::hash_t> *out) const override {
    std::string dbValue{};
    rocksdb::Status s = _db->Get(rocksdb::ReadOptions(),
                                 _columnHandle,
                                 tipsName<stored_block_t>(),
                                 &dbValue);
    if (!s.ok()) {
      if (s.IsNotFound()) return false;
      throw db::DbError(s.ToString());
    }

    *out = deserializeTipsFromRocks<stored_block_t>(dbValue);
    return true;
  }

 private:
  rocksdb::DB* _db;
  cf_handle_t* _columnHandle;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_TIPS_REPOSITORY_ROCKS_HPP_
