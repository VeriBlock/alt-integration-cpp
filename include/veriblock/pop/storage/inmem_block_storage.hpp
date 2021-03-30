// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_INMEM_BLOCK_STORAGE_HPP
#define VERIBLOCK_POP_CPP_INMEM_BLOCK_STORAGE_HPP

#include "block_reader.hpp"

#include "block_batch.hpp"
#include <veriblock/pop/storage/stored_block_index.hpp>

namespace altintegration {

/**
 * @class InmemBlockProvider
 * In-memory block storage, used in tests.
 * @private
 */
struct InmemBlockProvider {
  template <typename T>
  using M = std::unordered_map<typename T::hash_t,
                               std::unique_ptr<StoredBlockIndex<T>>>;

  template <typename T>
  M<T>& getBlocks();

  template <typename T>
  typename T::hash_t getTip() const;

  template <typename T>
  void save(const M<T>& m) {
    getBlocks<T>() = m;
  }

  template <typename T>
  std::vector<std::unique_ptr<StoredBlockIndex<T>>> load() {
    std::vector<std::unique_ptr<StoredBlockIndex<T>>> ret;
    auto& m = getBlocks<T>();
    ret.reserve(m.size());

    for (auto& b : m) {
      auto& bi = b.second;
      ret.push_back(std::move(bi));
    }

    return ret;
  }

  M<BtcBlock> btc;
  M<VbkBlock> vbk;
  M<AltBlock> alt;

  BtcBlock::hash_t btcTip;
  VbkBlock::hash_t vbkTip;
  AltBlock::hash_t altTip;
};

// clang-format off
template <> inline InmemBlockProvider::M<BtcBlock>& InmemBlockProvider::getBlocks() { return btc; }
template <> inline InmemBlockProvider::M<VbkBlock>& InmemBlockProvider::getBlocks() { return vbk; }
template <> inline InmemBlockProvider::M<AltBlock>& InmemBlockProvider::getBlocks() { return alt; }
template <> inline BtcBlock::hash_t InmemBlockProvider::getTip<BtcBlock>() const { return btcTip; }
template <> inline VbkBlock::hash_t InmemBlockProvider::getTip<VbkBlock>() const { return vbkTip; }
template <> inline AltBlock::hash_t InmemBlockProvider::getTip<AltBlock>() const { return altTip; }
// clang-format on

//! @private
struct InmemBlockBatch : public BlockBatch {
  InmemBlockBatch(InmemBlockProvider& storage) : storage_(storage) {}
  ~InmemBlockBatch() override = default;

  void writeBlock(const StoredBlockIndex<BtcBlock>& value) override {
    storage_.btc[value.header->getHash()] = make_unique<StoredBlockIndex<BtcBlock>>(value);
  }

  void writeBlock(const StoredBlockIndex<VbkBlock>& value) override {
    storage_.vbk[value.header->getHash()] =
        make_unique<StoredBlockIndex<VbkBlock>>(value);
  }

  void writeBlock(const StoredBlockIndex<AltBlock>& value) override {
    storage_.alt[value.header->getHash()] =
        make_unique<StoredBlockIndex<AltBlock>>(value);
  }

  void writeTip(const StoredBlockIndex<BtcBlock>& value) override {
    storage_.btcTip = value.header->getHash();
  }

  void writeTip(const StoredBlockIndex<VbkBlock>& value) override {
    storage_.vbkTip = value.header->getHash();
  }

  void writeTip(const StoredBlockIndex<AltBlock>& value) override {
    storage_.altTip = value.header->getHash();
  }

 private:
  InmemBlockProvider& storage_;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_INMEM_BLOCK_STORAGE_HPP
