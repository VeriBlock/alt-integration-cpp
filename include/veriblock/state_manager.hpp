#ifndef ALT_INTEGRATION_VERIBLOCK_STATE_MANAGER_H
#define ALT_INTEGRATION_VERIBLOCK_STATE_MANAGER_H

#include <memory>
#include <string>

#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/storage/block_repository.hpp"

namespace VeriBlock {

// BlockRepositoryManager should have an interface described below
// class BlockRepositoryManager:
// using status_t = (any status type, for example bool)
// template <typename Block_t>  using block_repo_t =
// (Block repository type that inherits from the BlockRepository);

template <typename BlockRepositoryManager>
class StateManager {
  template <typename Block_t>
  using block_repo_t = BlockRepository<BlockIndex<Block_t>>;

  template <typename Block_t>
  using cursor_t = typename block_repo_t<Block_t>::cursor_t;

  using status_t = typename BlockRepositoryManager::status_t;

  BlockRepositoryManager database;

 public:
  StateManager(const std::string& name) : database(name) { database.open(); }

  status_t commit() { return database.flush(); }

  status_t wipeRepos() { return database.clear(); }

  bool putBtcBlock(const BlockIndex<BtcBlock>& block) {
    return database.getBtcRepo()->put(block);
  }

  bool putVbkBlock(const BlockIndex<VbkBlock>& block) {
    return database.getVbkRepo()->put(block);
  }

  std::shared_ptr<cursor_t<BtcBlock>> getBtcCursor() {
    return database.getBtcRepo()->newCursor();
  }
  std::shared_ptr<cursor_t<VbkBlock>> getVbkCursor() {
    return database.getVbkRepo()->newCursor();
  };
};

}  // namespace VeriBlock

#endif  // ! ALT_INTEGRATION_VERIBLOCK
