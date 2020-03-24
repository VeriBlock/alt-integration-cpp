#ifndef ALT_INTEGRATION_VERIBLOCK_STATE_MANAGER_H
#define ALT_INTEGRATION_VERIBLOCK_STATE_MANAGER_H

#include <memory>
#include <string>

#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/payloads.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/storage/payloads_repository.hpp"

namespace altintegration {

template <typename RepositoryManager>
class StateManager;

class StateChange {
  friend class StateManager<void>;

 public:
  StateChange(std::shared_ptr<PayloadsRepository> payoadsAltRepo)
      : payloadsAltBatch(payoadsAltRepo->newBatch()) {}

  void clear() { payloadsAltBatch->clear(); }

  void savePayloads(const Payloads& payloads) {
    auto hash = payloads.alt.containing.getHash();
    payloadsAltBatch->put(Slice<const uint8_t>(hash.data(), hash.size()),
                          payloads);
  }

  void removePayloads(const Payloads& payloads) {
    auto hash = payloads.alt.containing.getHash();
    payloadsAltBatch->removeByHash(
        Slice<const uint8_t>(hash.data(), hash.size()));
  }

 private:
  std::unique_ptr<PayloadsWriteBatch> payloadsAltBatch;

  void commit() { payloadsAltBatch->commit(); }
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
  StateManager(const std::string& dbPath) : database(dbPath) {
    database.open();
  }

  std::unique_ptr<StateChange> newChange() {
    return std::unique_ptr<StateChange>(
        new StateChange(database.getAltPayloadsRepo()));
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

  RepositoryManager& getManager() { return database; }
};

}  // namespace altintegration

#endif  // ! ALT_INTEGRATION_VERIBLOCK
