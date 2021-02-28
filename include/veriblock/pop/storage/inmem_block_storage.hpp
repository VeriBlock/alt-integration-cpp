// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_INMEM_BLOCK_STORAGE_HPP
#define VERIBLOCK_POP_CPP_INMEM_BLOCK_STORAGE_HPP

#include "block_reader.hpp"

#include "block_batch.hpp"

namespace altintegration {

/**
 * @class InmemBlockProvider
 * In-memory block storage, used in tests.
 * @private
 */
struct InmemBlockProvider {
  template <typename T>
  using M =
      std::unordered_map<typename T::hash_t, std::unique_ptr<BlockIndex<T>>>;

  template <typename T>
  M<T>& getBlocks();

  template <typename T>
  typename T::hash_t getTip() const;

  template <typename T>
  void save(const M<T>& m) {
    getBlocks<T>() = m;
  }

  template <typename T>
  std::vector<std::unique_ptr<BlockIndex<T>>> load() {
    std::vector<std::unique_ptr<BlockIndex<T>>> ret;
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

  void writeBlock(const BlockIndex<BtcBlock>& value) override {
    storage_.btc[value.getHash()] = make_unique<BlockIndex<BtcBlock>>(value);
  }

  void writeBlock(const BlockIndex<VbkBlock>& value) override {
    storage_.vbk[value.getHash()] = make_unique<BlockIndex<VbkBlock>>(value);
  }

  void writeBlock(const BlockIndex<AltBlock>& value) override {
    storage_.alt[value.getHash()] = make_unique<BlockIndex<AltBlock>>(value);
  }

  void writeTip(const BlockIndex<BtcBlock>& value) override {
    storage_.btcTip = value.getHash();
  }

  void writeTip(const BlockIndex<VbkBlock>& value) override {
    storage_.vbkTip = value.getHash();
  }

  void writeTip(const BlockIndex<AltBlock>& value) override {
    storage_.altTip = value.getHash();
  }

 private:
  InmemBlockProvider& storage_;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_INMEM_BLOCK_STORAGE_HPP
