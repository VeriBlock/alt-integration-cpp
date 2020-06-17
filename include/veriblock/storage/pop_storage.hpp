#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_

#include <map>
#include <veriblock/entities/payloads.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/storage/block_repository_inmem.hpp>
#include <veriblock/storage/endorsement_storage.hpp>
#include <veriblock/storage/blocks_storage.hpp>
#include <veriblock/storage/storage_exceptions.hpp>

namespace altintegration {

class PopStorage : public EndorsementStorage<AltEndorsement>,
                   public EndorsementStorage<VbkEndorsement>,
                   public EndorsementStorage<DummyEndorsement>,
                   public BlocksStorage<BlockIndex<BtcBlock>>,
                   public BlocksStorage<BlockIndex<VbkBlock>>,
                   public BlocksStorage<BlockIndex<AltBlock>> {
 public:
  virtual ~PopStorage() = default;
  PopStorage() {}

  template <typename Endorsements>
  DummyEndorsement loadEndorsements(
      const typename DummyEndorsement::id_t&) const {
    return {};
  }

  template <typename Endorsements,
            typename = typename std::enable_if<
                !std::is_same<Endorsements, DummyEndorsement>::value>::type>
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
  void saveEndorsements(const Endorsements&
          endorsements) {
    bool ret = EndorsementStorage<Endorsements>::erepo_->put(endorsements);
    if (!ret) {
      throw BadIOException(fmt::sprintf("Failed to write endorsements: %s",
                                        endorsements.toPrettyString(0)));
    }
  }

  template <typename StoredBlock>
  std::multimap<typename StoredBlock::height_t, std::shared_ptr<StoredBlock>> loadBlocks() const {
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

  template <typename StoredBlock>
  void saveBlocks(
      const std::unordered_map<typename StoredBlock::prev_hash_t,
                               std::shared_ptr<StoredBlock>>& blocks) {

    auto batch = BlocksStorage<StoredBlock>::brepo_->newBatch();
    if (batch == nullptr) {
      throw BadIOException("Cannot create BlockRepository write batch");
    }

    for (const auto& block : blocks) {
      auto& index = *(block.second);
      batch->put(index);

      for (const auto& e : index.containingEndorsements) {
        saveEndorsements<typename StoredBlock::endorsement_t>(*e.second);
      }
    }
    batch->commit();
  }

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