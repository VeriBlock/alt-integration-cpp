// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_INMEM_BLOCK_STORAGE_HPP
#define VERIBLOCK_POP_CPP_INMEM_BLOCK_STORAGE_HPP

#include "block_batch_adaptor.hpp"

namespace altintegration {

/**
 * @class InmemBlockStorage
 * In-memory block storage, used in tests.
 */
struct InmemBlockStorage {
  template <typename T>
  using M =
      std::unordered_map<typename T::hash_t, std::shared_ptr<BlockIndex<T>>>;

  template <typename T>
  M<T>& getBlocks();

  template <typename T>
  typename T::hash_t getTip() const;

  template <typename T>
  void save(const M<T>& m) {
    getBlocks<T>() = m;
  }

  template <typename T>
  std::vector<BlockIndex<T>> load() {
    std::vector<BlockIndex<T>> ret;
    auto& m = getBlocks<T>();
    ret.reserve(m.size());

    for (auto& b : m) {
      auto& bi = b.second;
      ret.push_back(*bi);
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
template <> inline InmemBlockStorage::M<BtcBlock>& InmemBlockStorage::getBlocks() { return btc; }
template <> inline InmemBlockStorage::M<VbkBlock>& InmemBlockStorage::getBlocks() { return vbk; }
template <> inline InmemBlockStorage::M<AltBlock>& InmemBlockStorage::getBlocks() { return alt; }
template <> inline BtcBlock::hash_t InmemBlockStorage::getTip<BtcBlock>() const { return btcTip; }
template <> inline VbkBlock::hash_t InmemBlockStorage::getTip<VbkBlock>() const { return vbkTip; }
template <> inline AltBlock::hash_t InmemBlockStorage::getTip<AltBlock>() const { return altTip; }
// clang-format on

struct InmemBlockBatch : public BlockBatchAdaptor {
  InmemBlockBatch(InmemBlockStorage& storage) : storage_(storage) {}
  ~InmemBlockBatch() override = default;

  bool writeBlock(const BlockIndex<BtcBlock>& value) override {
    storage_.btc[value.getHash()] =
        std::make_shared<BlockIndex<BtcBlock>>(value);
    return true;
  }

  bool writeBlock(const BlockIndex<VbkBlock>& value) override {
    storage_.vbk[value.getHash()] =
        std::make_shared<BlockIndex<VbkBlock>>(value);
    return true;
  }

  bool writeBlock(const BlockIndex<AltBlock>& value) override {
    storage_.alt[value.getHash()] =
        std::make_shared<BlockIndex<AltBlock>>(value);
    return true;
  }

  bool writeTip(const BlockIndex<BtcBlock>& value) override {
    storage_.btcTip = value.getHash();
    return true;
  }

  bool writeTip(const BlockIndex<VbkBlock>& value) override {
    storage_.vbkTip = value.getHash();
    return true;
  }

  bool writeTip(const BlockIndex<AltBlock>& value) override {
    storage_.altTip = value.getHash();
    return true;
  }

 private:
  InmemBlockStorage& storage_;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_INMEM_BLOCK_STORAGE_HPP
