#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENTS_REPOSITORY_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENTS_REPOSITORY_ROCKS_HPP_

#include <rocksdb/db.h>

#include <memory>
#include <string>
#include <vector>

#include "veriblock/serde.hpp"
#include "veriblock/storage/endorsement_repository.hpp"

namespace VeriBlock {

//! column family type
using cf_handle_t = rocksdb::ColumnFamilyHandle;

template <typename Endorsement>
class EndorsementRepositoryRocks
    : public EndorsementRepositoryRocks<Endorsement> {
 public:
  using endorsement_t = Endorsement;
  using eid_t = typename Endorsement::id_t;
  using endorsed_hash_t = typename Endorsement::endorsed_hash_t;
  using containing_hash_t = typename Endorsement::containing_hash_t;
  using container_t = typename Endorsement::container_t;

 public:
  EndorsementRepositoryRocks(
      std::shared_ptr<rocksdb::DB> db,
      std::shared_ptr<cf_handle_t> endorsedBlockHashHandle,
      std::shared_ptr<cf_handle_t> endorsedIdHandle)
      : _db(std::move(db)),
        _endorsedBlockHashHandle(std::move(endorsedBlockHashHandle)),
        _endorsedIdHandle(std::move(endorsedIdHandle)) {}

  void remove(const eid_t& id) override {
    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;

    rocksdb::Slice key(reinterpret_cast<const char*>(id.data()), id.size());
    std::string outData{};

    rocksdb::Status s =
        _db->Get(rocksdb::ReadOptions(), _endorsedIdHandle.get(), key, outData);

    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }

    auto container = container_t::fromVbkEncoding(outData);
    s = _db->Delete(write_options, _endorsedIdHandle.get(), key);

    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }

    s = _db->Delete(
        write_options, _endorsedBlockHashHandle.get(), container.endorsedHash);

    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }
  }

  void remove(const container_t& container) override {
    remove(Endorsement::getId(container));
  }

  void put(const container_t& container) override {
    auto e = Endorsement::fromContainer(container);

    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s;

    rocksdb::Slice key(reinterpret_cast<const char*>(e.endorsedHash.data()),
                       e.endorsedHash.size());

    std::string valOut;
    s = _db->Get(write_options, _endorsedBlockHashHandle.get(), key, valOut);

    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }

    WriteStream w_stream;
    if (!s.IsNotFound()) {
      ReadStream r_stream(valOut);
      uint32_t vec_size = r_stream.readBE<uint32_t>();
      std::vector<id_t> ids(vec_size + 1);
      for (uint32_t i = 0; i < vec_size; ++i) {
        ids[i] = r_stream.readSlice(sizeof(eid_t));
      }
      ids[vec_size] = e.id;

      w_stream.writeBE<uint32_t>(ids.size());
      for (uint32_t i = 0; i < ids.size(); ++i) {
        w_stream.write(ids[i]);
      }
    } else {
      w_stream.writeBE<uint32_t>(1);
      w_stream.write(e.id);
    }

    rocksdb::Slice val(reinterpret_cast<const char*>(w_stream.data().data()),
                       w_stream.data().size());

    s = _db->Put(write_options, _endorsedBlockHashHandle.get(), key, val);

    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }

    std::vector<uint8_t> bytes = e.toVbkEncoding();
    key =
        rocksdb::Slice(reinterpret_cast<const char*>(e.id.data()), e.id.size());
    val = rocksdb::Slice(reinterpret_cast<const char*>(bytes.data()),
                         bytes.size());

    s = _db->Put(write_options, _endorsedIdHandle.get(), key, val);

    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }
  }

  std::vector<endorsement_t> get(
      const endorsed_hash_t& endorsedBlockHash) override {
    std::vector<endorsement_t> endorsements;

    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;
    rocksdb::Status s;

    rocksdb::Slice key(reinterpret_cast<const char*>(endorsedBlockHash.data()),
                       endorsedBlockHash.size());

    std::string valOut;
    s = _db->Get(write_options, _endorsedBlockHashHandle.get(), key, valOut);
    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }

    if (s.IsNotFound()) {
      return std::vector<endorsement_t>();
    }

    std::string endorsement_out;

    ReadStream r_stream(valOut);
    uint32_t endorsement_count = r_stream.readBE<uint32_t>();
    std::vector<endorsement_t> endorsements(endorsement_count);
    for (uint32_t i = 0; i < endorsement_count; ++i) {
      auto temp = r_stream.readSlice(sizeof(eid_t));
      key = rocksdb::Slice(reinterpret_cast<const char*>(temp.data()),
                           temp.size());
      _db->Get(write_options, _endorsedIdHandle.get(), key, endorsement_out);
      endorsements[i] = endorsement_t::fromVbkEncoding(endorsement_out);
    }

    return endorsements;
  }

 private:
  std::shared_ptr<rocksdb::DB> _db;
  std::shared_ptr<cf_handle_t> _endorsedBlockHashHandle;
  std::shared_ptr<cf_handle_t> _endorsedIdHandle;
};

}  // namespace VeriBlock

#endif
