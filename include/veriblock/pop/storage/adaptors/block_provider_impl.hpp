// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_BLOCK_PROVIDER_IMPL_HPP
#define VERIBLOCK_POP_CPP_STORAGE_ADAPTORS_BLOCK_PROVIDER_IMPL_HPP

#include <veriblock/pop/c/extern.h>

#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/storage/block_batch.hpp>
#include <veriblock/pop/storage/block_reader.hpp>

#include "storage_interface.hpp"

namespace altintegration {

namespace adaptors {

const char DB_BTC_BLOCK = 'Q';
const char DB_BTC_TIP = 'q';
const char DB_VBK_BLOCK = 'W';
const char DB_VBK_TIP = 'w';
const char DB_ALT_BLOCK = 'E';
const char DB_ALT_TIP = 'e';

template <typename block_t>
std::vector<uint8_t> tip_key();

template <>
std::vector<uint8_t> tip_key<AltBlock>();

template <>
std::vector<uint8_t> tip_key<VbkBlock>();

template <>
std::vector<uint8_t> tip_key<BtcBlock>();

template <typename block_t>
std::vector<uint8_t> block_key(const typename block_t::prev_hash_t& hash);

template <>
std::vector<uint8_t> block_key<AltBlock>(const AltBlock::prev_hash_t& hash);

template <>
std::vector<uint8_t> block_key<VbkBlock>(const VbkBlock::prev_hash_t& hash);

template <>
std::vector<uint8_t> block_key<BtcBlock>(const BtcBlock::prev_hash_t& hash);

template <typename BlockT>
struct BlockIteratorImpl : public BlockIterator<BlockT> {
  ~BlockIteratorImpl() override = default;

  BlockIteratorImpl(std::shared_ptr<StorageIterator> it) : it_(it) {}

  void next() override { it_->next(); }

  bool value(StoredBlockIndex<BlockT>& out) const override {
    std::vector<uint8_t> bytes;
    if (!it_->value(bytes)) {
      return false;
    }
    out = AssertDeserializeFromVbkEncoding<StoredBlockIndex<BlockT>>(bytes);
    return true;
  }

  bool key(typename BlockT::hash_t& out) const override {
    std::vector<uint8_t> bytes;
    if (!it_->key(bytes)) {
      return false;
    }
    // remove prefix
    out = {bytes.begin() + 1, bytes.end()};
    return true;
  }

  bool valid() const override {
    static const uint8_t prefix = block_key<BlockT>({})[0];

    std::vector<uint8_t> key;
    return it_->valid() && it_->key(key) && !key.empty() && key[0] == prefix;
  }

  void seek_start() override {
    static const uint8_t prefix = block_key<BlockT>({})[0];

    it_->seek({prefix});
  }

 private:
  std::shared_ptr<StorageIterator> it_;
};

struct BlockReaderImpl : public BlockReader {
  ~BlockReaderImpl() override = default;

  BlockReaderImpl(Storage& storage) : storage_(storage) {}

  bool getAltTip(AltBlock::hash_t& out) const override {
    std::vector<uint8_t> bytes_out;
    if (!storage_.read(tip_key<AltBlock>(), bytes_out)) {
      return false;
    }
    out = bytes_out;
    return true;
  }
  bool getVbkTip(VbkBlock::hash_t& out) const override {
    std::vector<uint8_t> bytes_out;
    if (!storage_.read(tip_key<VbkBlock>(), bytes_out)) {
      return false;
    }
    out = bytes_out;
    return true;
  }
  bool getBtcTip(BtcBlock::hash_t& out) const override {
    std::vector<uint8_t> bytes_out;
    if (!storage_.read(tip_key<BtcBlock>(), bytes_out)) {
      return false;
    }
    out = bytes_out;
    return true;
  }

  bool getBlock(const AltBlock::prev_hash_t& hash,
                StoredBlockIndex<AltBlock>& out) const override {
    return getBlock<AltBlock>(hash, out);
  }

  bool getBlock(const VbkBlock::prev_hash_t& hash,
                StoredBlockIndex<VbkBlock>& out) const override {
    return getBlock<VbkBlock>(hash, out);
  }

  bool getBlock(const BtcBlock::prev_hash_t& hash,
                StoredBlockIndex<BtcBlock>& out) const override {
    return getBlock<BtcBlock>(hash, out);
  }

  std::shared_ptr<BlockIterator<AltBlock>> getAltBlockIterator()
      const override {
    return std::make_shared<BlockIteratorImpl<AltBlock>>(
        storage_.generateIterator());
  }
  std::shared_ptr<BlockIterator<VbkBlock>> getVbkBlockIterator()
      const override {
    return std::make_shared<BlockIteratorImpl<VbkBlock>>(
        storage_.generateIterator());
  }
  std::shared_ptr<BlockIterator<BtcBlock>> getBtcBlockIterator()
      const override {
    return std::make_shared<BlockIteratorImpl<BtcBlock>>(
        storage_.generateIterator());
  }

 private:
  template <typename block_t>
  bool getBlock(const typename block_t::prev_hash_t& hash,
                StoredBlockIndex<block_t>& out) const {
    std::vector<uint8_t> bytes_out;
    if (!storage_.read(block_key<block_t>(hash), bytes_out)) {
      return false;
    }
    out =
        AssertDeserializeFromVbkEncoding<StoredBlockIndex<block_t>>(bytes_out);
    return true;
  }

  Storage& storage_;
};

struct BlockBatchImpl : public BlockBatch {
  ~BlockBatchImpl() override = default;

  BlockBatchImpl(WriteBatch& batch) : batch_(batch) {}

  void writeBlock(const AltBlock::prev_hash_t& hash,
                  const StoredBlockIndex<AltBlock>& blk) override {
    batch_.write(block_key<AltBlock>(hash), SerializeToVbkEncoding(blk));
  }

  void writeBlock(const VbkBlock::prev_hash_t& hash,
                  const StoredBlockIndex<VbkBlock>& blk) override {
    batch_.write(block_key<VbkBlock>(hash), SerializeToVbkEncoding(blk));
  }

  void writeBlock(const BtcBlock::prev_hash_t& hash,
                  const StoredBlockIndex<BtcBlock>& blk) override {
    batch_.write(block_key<BtcBlock>(hash), SerializeToVbkEncoding(blk));
  }

  void writeTip(const AltBlock::hash_t& hash) override {
    batch_.write(tip_key<AltBlock>(), hash);
  }

  void writeTip(const VbkBlock::hash_t& hash) override {
    batch_.write(tip_key<VbkBlock>(), hash.asVector());
  }

  void writeTip(const BtcBlock::hash_t& hash) override {
    batch_.write(tip_key<BtcBlock>(), hash.asVector());
  }

 private:
  WriteBatch& batch_;
};

}  // namespace adaptors

}  // namespace altintegration

#endif