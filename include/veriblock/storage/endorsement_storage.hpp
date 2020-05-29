// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_

#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/blockchain/command_group.hpp>
#include <veriblock/entities/payloads.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/storage/payloads_repository_inmem.hpp>

namespace altintegration {

template <typename Payloads, typename Tree>
class EndorsementStorage {
  using payloads_t = Payloads;
  using block_t = typename Tree::block_t;
  using eid_t = typename Payloads::id_t;
  using containing_hash_t = typename block_t::hash_t;

 public:
  virtual ~EndorsementStorage() = default;

  EndorsementStorage()
      : prepo_(
            std::move(std::make_shared<PayloadsRepositoryInmem<Payloads>>())) {}

  virtual bool getCommands(Tree& tree,
                           const eid_t& id,
                           CommandGroup& commandGroup) {
    payloads_t payload{};
    bool ret = prepo_->get(id, &payload);
    if (!ret) return false;

    std::vector<CommandPtr> commands{};
    tree.payloadsToCommands(payload, commands);
    commandGroup.id = id;
    commandGroup.commands = commands;
    return true;
  }

  PayloadsRepository<Payloads>& payloads() { return *prepo_; }
  const PayloadsRepository<Payloads>& payloads() const { return *prepo_; }

 private:
  std::shared_ptr<PayloadsRepository<Payloads>> prepo_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_