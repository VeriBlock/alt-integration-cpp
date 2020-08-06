// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/storage/block_repository.hpp>
#include <veriblock/storage/db_error.hpp>
#include <veriblock/storage/payloads_repository.hpp>
#include <veriblock/storage/tips_repository.hpp>

namespace altintegration {

class PopStorage {
 public:
  virtual ~PopStorage() = default;

  template <typename Block>
  BlockRepository<Block>& getBlockRepo();

  template <typename Block>
  const BlockRepository<Block>& getBlockRepo() const;

  template <typename Block>
  TipsRepository<Block>& getTipsRepo();

  template <typename Block>
  const TipsRepository<Block>& getTipsRepo() const;

  template <typename StoredBlock>
  std::multimap<typename StoredBlock::height_t, std::shared_ptr<StoredBlock>>
  loadBlocks() {
    auto& repo = getBlockRepo<StoredBlock>();
    auto cursor = repo.newCursor();
    VBK_ASSERT(cursor && "can not create cursor");
    cursor->seekToFirst();
    std::multimap<typename StoredBlock::height_t, std::shared_ptr<StoredBlock>>
        blocks{};
    while (cursor->isValid()) {
      auto block = cursor->value();
      blocks.insert({block.getHeight(), std::make_shared<StoredBlock>(block)});
      cursor->next();
    }
    return blocks;
  }

  template <typename StoredBlock>
  void saveBlocks(
      const std::unordered_map<typename StoredBlock::prev_hash_t,
                               std::shared_ptr<StoredBlock>>& blocks) {
    auto& repo = getBlockRepo<StoredBlock>();
    auto batch = repo.newBatch();
    VBK_ASSERT(batch && "can not create batch");

    for (const auto& block : blocks) {
      auto& index = *(block.second);
      batch->put(index);
    }
    batch->commit();
  }

  template <typename StoredBlock>
  std::pair<typename StoredBlock::height_t, typename StoredBlock::hash_t>
  loadTip() const {
    auto& repo = getTipsRepo<StoredBlock>();
    std::pair<typename StoredBlock::height_t, typename StoredBlock::hash_t> out;
    repo.get(&out);
    return out;
  }

  template <typename StoredBlock>
  void saveTip(const StoredBlock& tip) {
    auto& repo = getTipsRepo<StoredBlock>();
    repo.put(tip);
  }

 protected:
  PopStorage() = default;

  std::shared_ptr<BlockRepository<BlockIndex<BtcBlock>>> _brepoBtc;
  std::shared_ptr<BlockRepository<BlockIndex<VbkBlock>>> _brepoVbk;
  std::shared_ptr<BlockRepository<BlockIndex<AltBlock>>> _brepoAlt;
  std::shared_ptr<TipsRepository<BlockIndex<BtcBlock>>> _trepoBtc;
  std::shared_ptr<TipsRepository<BlockIndex<VbkBlock>>> _trepoVbk;
  std::shared_ptr<TipsRepository<BlockIndex<AltBlock>>> _trepoAlt;
  std::shared_ptr<PayloadsRepository<VbkEndorsement>> _erepoVbk;
  std::shared_ptr<PayloadsRepository<AltEndorsement>> _erepoAlt;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_