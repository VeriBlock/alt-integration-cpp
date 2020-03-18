#ifndef ALT_INTEGRATION_VERIBLOCK_STATE_MANAGER_H
#define ALT_INTEGRATION_VERIBLOCK_STATE_MANAGER_H

#include <memory>
#include <string>

#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/endorsement.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/storage/block_repository.hpp"
#include "veriblock/storage/endorsement_repository.hpp"

namespace AltIntegrationLib {

class StateChange {
 public:
  StateChange(
      std::shared_ptr<BlockRepository<BlockIndex<BtcBlock>>> btcRepo,
      std::shared_ptr<BlockRepository<BlockIndex<VbkBlock>>> vbkRepo,
      std::shared_ptr<EndorsementRepository<BtcEndorsement>> btcEndorsementRepo,
      std::shared_ptr<EndorsementRepository<VbkEndorsement>> vbkEndorsementRepo)
      : btcBlockBatch(btcRepo->newBatch()),
        vbkBlockBatch(vbkRepo->newBatch()),
        btcEndorsementBatch(btcEndorsementRepo->newBatch()),
        vbkEndorsementBatch(vbkEndorsementRepo->newBatch()) {}

  void clear() {
    btcBlockBatch->clear();
    vbkBlockBatch->clear();
    btcEndorsementBatch->clear();
    vbkEndorsementBatch->clear();
  }

  void putBtcBlock(const BlockIndex<BtcBlock>& block) {
    btcBlockBatch->put(block);
  }

  void removeBtcBlock(const BtcBlock::hash_t& hash) {
    btcBlockBatch->removeByHash(hash);
  }

  void putVbkBlock(const BlockIndex<VbkBlock>& block) {
    vbkBlockBatch->put(block);
  }

  void removeVbkBlock(const VbkBlock::hash_t& hash) {
    vbkBlockBatch->removeByHash(hash);
  }

  void putBtcEndorsement(const BtcEndorsement& endorsement) {
    btcEndorsementBatch->put(endorsement);
  }

  void putBtcEndorsement(const BtcEndorsement::container_t& container) {
    btcEndorsementBatch->put(container);
  }

  void removeBtcEndorsement(const BtcEndorsement& endorsement) {
    btcEndorsementBatch->remove(endorsement.id);
  }

  void removeBtcEndorsement(const BtcEndorsement::container_t& container) {
    btcEndorsementBatch->remove(container);
  }

  void putVbkEndorsement(const VbkEndorsement& endorsement) {
    vbkEndorsementBatch->put(endorsement);
  }

  void putVbkEndorsement(const VbkEndorsement::container_t& container) {
    vbkEndorsementBatch->put(container);
  }

  void removeVbkEndorsement(const VbkEndorsement& endorsement) {
    vbkEndorsementBatch->remove(endorsement.id);
  }

  void removeVbkEndorsement(const VbkEndorsement::container_t& container) {
    vbkEndorsementBatch->remove(container);
  }

  void commit() {
    btcBlockBatch->commit();
    vbkBlockBatch->commit();
    btcEndorsementBatch->commit();
    vbkEndorsementBatch->commit();
  }

 private:
  std::unique_ptr<BlockWriteBatch<BlockIndex<BtcBlock>>> btcBlockBatch;
  std::unique_ptr<BlockWriteBatch<BlockIndex<VbkBlock>>> vbkBlockBatch;

  std::unique_ptr<EndorsementWriteBatch<BtcEndorsement>> btcEndorsementBatch;
  std::unique_ptr<EndorsementWriteBatch<VbkEndorsement>> vbkEndorsementBatch;
};

// BlockRepositoryManager should have an interface described below
// class BlockRepositoryManager:
// using status_t = (any status type, for example bool)
// template <typename Block_t>  using block_repo_t =
// (Block repository type that inherits from the BlockRepository);

template <typename RepositoryManager>
class StateManager {
  template <typename Block_t>
  using block_repo_t = BlockRepository<BlockIndex<Block_t>>;

  template <typename Block_t>
  using cursor_t = typename block_repo_t<Block_t>::cursor_t;

  using status_t = typename RepositoryManager::status_t;

  RepositoryManager database;

 public:
  StateManager(const std::string& name) : database(name) { database.open(); }

  std::unique_ptr<StateChange> newChange() {
    return std::unique_ptr<StateChange>(
        new StateChange(database.getBtcRepo(),
                        database.getVbkRepo(),
                        database.getBtcEndorsementRepo(),
                        database.getVbkEndorsementRepo()));
  }

  status_t commit(StateChange& change) {
    change.commit();
    return database.flush();
  }

  status_t wipeRepos() {
    status_t status = database.clear();
    return status;
  }

  std::shared_ptr<cursor_t<BtcBlock>> getBtcCursor() {
    return database.getBtcRepo()->newCursor();
  }

  std::shared_ptr<cursor_t<VbkBlock>> getVbkCursor() {
    return database.getVbkRepo()->newCursor();
  }
};

}  // namespace AltIntegrationLib

#endif  // ! ALT_INTEGRATION_VERIBLOCK
