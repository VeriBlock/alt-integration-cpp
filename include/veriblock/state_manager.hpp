#ifndef ALT_INTEGRATION_VERIBLOCK_STATE_MANAGER_H
#define ALT_INTEGRATION_VERIBLOCK_STATE_MANAGER_H

#include <memory>
#include <string>

#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/endorsement.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/storage/block_repository.hpp"

namespace VeriBlock {

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
  std::unique_ptr<WriteBatch<BlockIndex<BtcBlock>>> btcBlockBatch;
  std::unique_ptr<WriteBatch<BlockIndex<VbkBlock>>> vbkBlockBatch;

 public:
  StateManager(const std::string& name) : database(name) {
    database.open();
    btcBlockBatch = database.getBtcRepo()->newBatch();
    vbkBlockBatch = database.getVbkRepo()->newBatch();
  }

  status_t commit() {
    btcBlockBatch->commit();
    vbkBlockBatch->commit();
    return database.flush();
  }

  status_t wipeRepos() {
    status_t status = database.clear();
    btcBlockBatch = database.getBtcRepo()->newBatch();
    vbkBlockBatch = database.getVbkRepo()->newBatch();
    return status;
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
    database.getBtcEndorsementRepo()->put(endorsement);
  }

  void putVbkEndorsement(const VbkEndorsement& endorsement) {
    database.getVbkEndorsementRepo()->put(endorsement);
  }

  std::shared_ptr<cursor_t<BtcBlock>> getBtcCursor() {
    return database.getBtcRepo()->newCursor();
  }

  std::shared_ptr<cursor_t<VbkBlock>> getVbkCursor() {
    return database.getVbkRepo()->newCursor();
  }
};

}  // namespace VeriBlock

#endif  // ! ALT_INTEGRATION_VERIBLOCK
