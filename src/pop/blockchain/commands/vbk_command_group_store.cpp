// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/blockchain/blockchain_util.hpp>
#include <veriblock/pop/blockchain/commands/vbk_command_group_store.hpp>
#include <veriblock/pop/exceptions/state_corrupted.hpp>

namespace altintegration {

std::unique_ptr<CommandGroup> VbkCommandGroupStore::getCommand(
    const BlockIndex<VbkBlock>& block,
    const VTB::id_t& id,
    ValidationState& state) {
  VTB vtb;
  if (!_payloadStore.getVTB(id, vtb, state)) {
    throw StateCorruptedException(block, state);
  }

  auto cg = make_unique<CommandGroup>();
  payloadToCommandGroup(_tree, vtb, block.getHash().asVector(), *cg);
  return cg;
}

std::unique_ptr<VbkCommandGroupStore::command_groups_t>
VbkCommandGroupStore::getCommands(const BlockIndex<VbkBlock>& block,
                                  ValidationState& state) {
  const auto& vtb_ids = block.getPayloadIds<VTB>();

  auto groups = make_unique<command_groups_t>();
  groups->reserve(vtb_ids.size());

  for (const auto& id : vtb_ids) {
    auto cg = getCommand(block, id, state);
    if (!cg) {
      // invalid payload, the state has been set for us.
      return nullptr;
    }
    groups->push_back(std::move(cg));
  }
  return groups;
}

}  // namespace altintegration
