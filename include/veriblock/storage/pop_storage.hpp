#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_

#include <map>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/storage/block_repository.hpp>
#include <veriblock/storage/payloads_repository.hpp>
#include <veriblock/storage/storage_exceptions.hpp>
#include <veriblock/storage/tips_repository.hpp>
#include <utility>

namespace altintegration {

class PopStorage {
 public:
  virtual ~PopStorage() = default;
  PopStorage(std::shared_ptr<BlockRepository<BlockIndex<BtcBlock>>> brepoBtc,
             std::shared_ptr<BlockRepository<BlockIndex<VbkBlock>>> brepoVbk,
             std::shared_ptr<BlockRepository<BlockIndex<AltBlock>>> brepoAlt,
             std::shared_ptr<TipsRepository<BlockIndex<BtcBlock>>> trepoBtc,
             std::shared_ptr<TipsRepository<BlockIndex<VbkBlock>>> trepoVbk,
             std::shared_ptr<TipsRepository<BlockIndex<AltBlock>>> trepoAlt,
             std::shared_ptr<PayloadsRepository<VbkEndorsement>> erepoVbk,
             std::shared_ptr<PayloadsRepository<AltEndorsement>> erepoAlt)
      : _brepoBtc(brepoBtc),
        _brepoVbk(brepoVbk),
        _brepoAlt(brepoAlt),
        _trepoBtc(trepoBtc),
        _trepoVbk(trepoVbk),
        _trepoAlt(trepoAlt),
        _erepoVbk(erepoVbk),
        _erepoAlt(erepoAlt) {}

  template <typename Block>
  BlockRepository<Block>& getBlockRepo();

  template <typename Block>
  const BlockRepository<Block>& getBlockRepo() const;

  template <typename Block>
  TipsRepository<Block>& getTipsRepo();

  template <typename Block>
  const TipsRepository<Block>& getTipsRepo() const;

  template <typename Endorsements>
  PayloadsRepository<Endorsements>& getEndorsementsRepo();

  template <typename Endorsements>
  const PayloadsRepository<Endorsements>& getEndorsementsRepo() const;

  template <typename Endorsements>
  Endorsements loadEndorsements(const typename Endorsements::id_t& eid) const {
    Endorsements endorsements;
    auto& repo = getEndorsementsRepo<Endorsements>();
    repo.get(eid, &endorsements);
    return endorsements;
  }

  template <typename Endorsements>
  void saveEndorsements(const Endorsements& endorsements) {
    auto& repo = getEndorsementsRepo<Endorsements>();
    repo.put(endorsements);
  }

  template <typename StoredBlock>
  std::multimap<typename StoredBlock::height_t, std::shared_ptr<StoredBlock>>
  loadBlocks() {
    auto& repo = getBlockRepo<StoredBlock>();
    auto cursor = repo.newCursor();
    if (cursor == nullptr) {
      throw BadIOException("Cannot create BlockRepository cursor");
    }
    cursor->seekToFirst();
    std::multimap<typename StoredBlock::height_t, std::shared_ptr<StoredBlock>>
        blocks{};
    while (cursor->isValid()) {
      auto block = cursor->value();
      blocks.insert({block.height, std::make_shared<StoredBlock>(block)});
      cursor->next();
    }
    return blocks;
  }

  template <typename StoredBlock>
  void processSaveBlock(const StoredBlock&);

  template <typename StoredBlock>
  void saveBlocks(
      const std::unordered_map<typename StoredBlock::prev_hash_t,
                               std::shared_ptr<StoredBlock>>& blocks) {
    auto& repo = getBlockRepo<StoredBlock>();
    auto batch = repo.newBatch();
    if (batch == nullptr) {
      throw BadIOException("Cannot create BlockRepository write batch");
    }

    for (const auto& block : blocks) {
      auto& index = *(block.second);
      batch->put(index);
      processSaveBlock(index);
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