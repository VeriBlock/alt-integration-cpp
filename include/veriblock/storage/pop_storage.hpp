#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_

#include <map>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/storage/block_repository_inmem.hpp>
#include <veriblock/storage/blocks_storage.hpp>
#include <veriblock/storage/endorsement_storage.hpp>
#include <veriblock/storage/storage_exceptions.hpp>

namespace altintegration {

class PopStorage : public EndorsementStorage<AltEndorsement>,
                   public EndorsementStorage<VbkEndorsement>,
                   public BlocksStorage<BlockIndex<BtcBlock>>,
                   public BlocksStorage<BlockIndex<VbkBlock>>,
                   public BlocksStorage<BlockIndex<AltBlock>> {
 public:
  virtual ~PopStorage() = default;
  PopStorage() {}

  template <typename Endorsements>
  Endorsements loadEndorsements(const typename Endorsements::id_t& eid) const {
    Endorsements endorsements;
    bool ret =
        EndorsementStorage<Endorsements>::erepo_->get(eid, &endorsements);
    if (!ret) {
      throw StateCorruptedException(
          fmt::sprintf("Failed to read endorsements id={%s}", eid.toHex()));
    }
    return endorsements;
  }

  template <typename Endorsements>
  void saveEndorsements(const Endorsements& endorsements) {
    bool ret = EndorsementStorage<Endorsements>::erepo_->put(endorsements);
    if (!ret) {
      throw BadIOException(fmt::sprintf("Failed to write endorsements: %s",
                                        endorsements.toPrettyString(0)));
    }
  }

  template <typename StoredBlock>
  std::multimap<typename StoredBlock::height_t, std::shared_ptr<StoredBlock>>
  loadBlocks() const {
    auto cursor = BlocksStorage<StoredBlock>::brepo_->newCursor();
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

  // realisation in the alt_block_tree, vbk_block_tree
  template <typename StoredBlock>
  void saveBlocks(
      const std::unordered_map<typename StoredBlock::prev_hash_t,
                               std::shared_ptr<StoredBlock>>& blocks);

  template <typename StoredBlock>
  std::pair<typename StoredBlock::height_t, typename StoredBlock::hash_t>
  loadTip() const {
    return BlocksStorage<StoredBlock>::loadTip();
  }

  template <typename StoredBlock>
  void saveTip(const StoredBlock& tip) {
    BlocksStorage<StoredBlock>::saveTip(tip);
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_