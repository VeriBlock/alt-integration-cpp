#ifndef ALT_INTEGRATION_VERIBLOCK_STATE_MANAGER_H
#define ALT_INTEGRATION_VERIBLOCK_STATE_MANAGER_H

#include <memory>
#include <string>

#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/storage/block_repository.hpp"
#include "veriblock/storage/block_repository_rocks_manager.hpp"

namespace VeriBlock {

// BlockRepositoryManager should have an interface described below
// class BlockRepositoryManager:
// using status_t = (any status type, for example bool)
// template <typename Block_t>  using block_repo_t =
// (Block repository type that inherits from the BlockRepository);

template <typename BlockRepositoryManager>
class StateManager {
  template <typename Block_t>
  using block_repo_t =
      typename BlockRepositoryManager::template block_repo_t<Block_t>;

  BlockRepositoryManager database;
  std::shared_ptr<block_repo_t<VbkBlock>> vbkRepo;

 public:
  StateManager(const std::string &name) : database(name) {}
};

}  // namespace VeriBlock

#endif  // ! ALT_INTEGRATION_VERIBLOCK
