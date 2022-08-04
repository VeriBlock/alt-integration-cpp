// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <veriblock/pop/blockchain/blockchain_util.hpp>
#include <veriblock/pop/blockchain/commands/alt_command_group_store.hpp>
#include <veriblock/pop/exceptions/state_corrupted.hpp>
#include <memory>
#include <utility>
#include <vector>


#include "veriblock/pop/algorithm.hpp"
#include "veriblock/pop/blockchain/block_index.hpp"
#include "veriblock/pop/blockchain/command_group.hpp"
#include "veriblock/pop/entities/altblock.hpp"
#include "veriblock/pop/entities/atv.hpp"
#include "veriblock/pop/entities/vbkblock.hpp"
#include "veriblock/pop/entities/vtb.hpp"

namespace altintegration {
class ValidationState;

template <typename Payload>
std::unique_ptr<CommandGroup> AltCommandGroupStore::getCommand(
    const BlockIndex<AltBlock>& block,
    const typename Payload::id_t& id,
    ValidationState& state) {
  Payload payload;
  if (!_payloadStore.getPayload(id, payload, state)) {
    throw StateCorruptedException(block, state);
  }

  auto cg = make_unique<CommandGroup>();
  payloadToCommandGroup(_tree, payload, block.getHash(), *cg);
  return cg;
}

template std::unique_ptr<CommandGroup>
AltCommandGroupStore::getCommand<VbkBlock>(const BlockIndex<AltBlock>& block,
                                           const typename VbkBlock::id_t& id,
                                           ValidationState& state);

template std::unique_ptr<CommandGroup> AltCommandGroupStore::getCommand<VTB>(
    const BlockIndex<AltBlock>& block,
    const typename VTB::id_t& id,
    ValidationState& state);

template std::unique_ptr<CommandGroup> AltCommandGroupStore::getCommand<ATV>(
    const BlockIndex<AltBlock>& block,
    const typename ATV::id_t& id,
    ValidationState& state);

std::unique_ptr<AltCommandGroupStore::command_groups_t>
AltCommandGroupStore::getCommands(const BlockIndex<AltBlock>& block,
                                  ValidationState& state) {
  const auto& atv_ids = block.getPayloadIds<ATV>();
  const auto& vtb_ids = block.getPayloadIds<VTB>();
  const auto& vbk_ids = block.getPayloadIds<VbkBlock>();

  auto groups = make_unique<command_groups_t>();
  groups->reserve(atv_ids.size() + vtb_ids.size() + vbk_ids.size());

  // order is important!
  // first, add VBK blocks, then VTBs, and then ATVs

  for (const auto& id : vbk_ids) {
    auto cg = getCommand<VbkBlock>(block, id, state);
    if (!cg) {
      // invalid payload, the state has been set for us.
      return nullptr;
    }
    groups->push_back(std::move(cg));
  }

  for (const auto& id : vtb_ids) {
    auto cg = getCommand<VTB>(block, id, state);
    if (!cg) {
      // invalid payload, the state has been set for us.
      return nullptr;
    }
    groups->push_back(std::move(cg));
  }

  for (const auto& id : atv_ids) {
    auto cg = getCommand<ATV>(block, id, state);
    if (!cg) {
      // invalid payload, the state has been set for us.
      return nullptr;
    }
    groups->push_back(std::move(cg));
  }

  return groups;
}

}  // namespace altintegration
