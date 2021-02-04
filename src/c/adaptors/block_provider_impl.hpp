// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ADAPTORS_BLOCK_PROVIDER_IMPL_BINDINGS
#define VERIBLOCK_POP_CPP_ADAPTORS_BLOCK_PROVIDER_IMPL_BINDINGS

#include "storage_interface.hpp"
#include "veriblock/c/extern.h"
#include "veriblock/serde.hpp"
#include "veriblock/storage/block_batch.hpp"
#include "veriblock/storage/block_reader.hpp"

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
std::vector<uint8_t> tip_key<altintegration::AltBlock>();

template <>
std::vector<uint8_t> tip_key<altintegration::VbkBlock>();

template <>
std::vector<uint8_t> tip_key<altintegration::BtcBlock>();

template <typename block_t>
std::vector<uint8_t> block_key(const typename block_t::hash_t& hash);

template <>
std::vector<uint8_t> block_key<altintegration::AltBlock>(
    const altintegration::AltBlock::hash_t& hash);

template <>
std::vector<uint8_t> block_key<altintegration::VbkBlock>(
    const altintegration::VbkBlock::hash_t& hash);

template <>
std::vector<uint8_t> block_key<altintegration::BtcBlock>(
    const altintegration::BtcBlock::hash_t& hash);

template <typename BlockT>
struct BlockIteratorImpl : public altintegration::BlockIterator<BlockT> {
  ~BlockIteratorImpl() override = default;

  BlockIteratorImpl(std::shared_ptr<StorageIterator> it) : it_(it) {}

  void next() override { it_->next(); }

  bool value(altintegration::BlockIndex<BlockT>& out) const override {
    std::vector<uint8_t> bytes;
    if (!it_->value(bytes)) {
      return false;
    }
    altintegration::ValidationState tmp;
    if (!altintegration::DeserializeFromVbkEncoding(bytes, out, tmp)) {
      return false;
    }
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
    static char prefix = block_key<BlockT>({})[0];

    std::vector<uint8_t> key;
    return it_->valid() && it_->key(key) && !key.empty() && key[0] == prefix;
  }

  void seek_start() override { it_->seek(block_key<BlockT>({})); }

 private:
  std::shared_ptr<StorageIterator> it_;
};

struct BlockReaderImpl : public altintegration::BlockReader {
  ~BlockReaderImpl() override = default;

  BlockReaderImpl(Storage& storage) : storage_(storage) {}

  bool getAltTip(altintegration::AltBlock::hash_t& out) const override {
    std::vector<uint8_t> bytes_out;
    if (!storage_.read(tip_key<altintegration::AltBlock>(), bytes_out)) {
      return false;
    }
    out = bytes_out;
    return true;
  }
  bool getVbkTip(altintegration::VbkBlock::hash_t& out) const override {
    std::vector<uint8_t> bytes_out;
    if (!storage_.read(tip_key<altintegration::VbkBlock>(), bytes_out)) {
      return false;
    }
    out = bytes_out;
    return true;
  }
  bool getBtcTip(altintegration::BtcBlock::hash_t& out) const override {
    std::vector<uint8_t> bytes_out;
    if (!storage_.read(tip_key<altintegration::BtcBlock>(), bytes_out)) {
      return false;
    }
    out = bytes_out;
    return true;
  }

  std::shared_ptr<altintegration::BlockIterator<altintegration::AltBlock>>
  getAltBlockIterator() const override {
    return std::make_shared<BlockIteratorImpl<altintegration::AltBlock>>(
        storage_.generateIterator());
  }
  std::shared_ptr<altintegration::BlockIterator<altintegration::VbkBlock>>
  getVbkBlockIterator() const override {
    return std::make_shared<BlockIteratorImpl<altintegration::VbkBlock>>(
        storage_.generateIterator());
  }
  std::shared_ptr<altintegration::BlockIterator<altintegration::BtcBlock>>
  getBtcBlockIterator() const override {
    return std::make_shared<BlockIteratorImpl<altintegration::BtcBlock>>(
        storage_.generateIterator());
  }

 private:
  Storage& storage_;
};

struct BlockBatchImpl : public altintegration::BlockBatch {
  ~BlockBatchImpl() override = default;

  BlockBatchImpl(WriteBatch& batch) : batch_(batch) {}

  void writeBlock(const altintegration::BlockIndex<altintegration::AltBlock>&
                      blk) override {
    batch_.write(block_key<altintegration::AltBlock>(blk.getHash()),
                 altintegration::SerializeToVbkEncoding(blk));
  }

  void writeBlock(const altintegration::BlockIndex<altintegration::VbkBlock>&
                      blk) override {
    batch_.write(block_key<altintegration::VbkBlock>(blk.getHash()),
                 altintegration::SerializeToVbkEncoding(blk));
  }

  void writeBlock(const altintegration::BlockIndex<altintegration::BtcBlock>&
                      blk) override {
    batch_.write(block_key<altintegration::BtcBlock>(blk.getHash()),
                 altintegration::SerializeToVbkEncoding(blk));
  }

  void writeTip(const altintegration::BlockIndex<altintegration::AltBlock>& blk)
      override {
    batch_.write(tip_key<altintegration::AltBlock>(), blk.getHash());
    batch_.write(block_key<altintegration::AltBlock>(blk.getHash()),
                 altintegration::SerializeToVbkEncoding(blk));
  }

  void writeTip(const altintegration::BlockIndex<altintegration::VbkBlock>& blk)
      override {
    batch_.write(tip_key<altintegration::VbkBlock>(), blk.getHash().asVector());
    batch_.write(block_key<altintegration::VbkBlock>(blk.getHash()),
                 altintegration::SerializeToVbkEncoding(blk));
  }

  void writeTip(const altintegration::BlockIndex<altintegration::BtcBlock>& blk)
      override {
    batch_.write(tip_key<altintegration::BtcBlock>(), blk.getHash().asVector());
    batch_.write(block_key<altintegration::BtcBlock>(blk.getHash()),
                 altintegration::SerializeToVbkEncoding(blk));
  }

 private:
  WriteBatch& batch_;
};

}  // namespace adaptors

#endif