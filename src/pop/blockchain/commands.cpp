// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "veriblock/pop/blockchain/alt_chain_params.hpp"
#include "veriblock/pop/blockchain/blockchain_util.hpp"
#include "veriblock/pop/blockchain/command.hpp"
#include "veriblock/pop/blockchain/command_group.hpp"
#include "veriblock/pop/blockchain/commands/addblock.hpp"
#include "veriblock/pop/blockchain/commands/addendorsement.hpp"
#include "veriblock/pop/blockchain/commands/addvtb.hpp"
#include "veriblock/pop/blockchain/commands/check_pubdata.hpp"
#include "veriblock/pop/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/pop/entities/atv.hpp"
#include "veriblock/pop/entities/endorsements.hpp"
#include "veriblock/pop/entities/publication_data.hpp"
#include "veriblock/pop/entities/vbkpoptx.hpp"
#include "veriblock/pop/entities/vbktx.hpp"
#include "veriblock/pop/entities/vtb.hpp"

namespace altintegration {
struct VbkBlock;

template <>
void payloadToCommands(AltBlockTree& tree,
                       const VbkBlock& pop,
                       const std::vector<uint8_t>& /* ignore */,
                       std::vector<CommandPtr>& cmds) {
  addBlock(tree.vbk(), pop, cmds);
}

template <>
void payloadToCommands(AltBlockTree& tree,
                       const VTB& pop,
                       const std::vector<uint8_t>& /* ignore */,
                       std::vector<CommandPtr>& cmds) {
  auto cmd = std::make_shared<AddVTB>(tree, pop);
  cmds.push_back(std::move(cmd));
}

template <>
void payloadToCommands(AltBlockTree& tree,
                       const ATV& pop,
                       const std::vector<uint8_t>& containingHash,
                       std::vector<CommandPtr>& cmds) {
  addBlock(tree.vbk(), pop.blockOfProof, cmds);

  std::vector<uint8_t> endorsedHash =
      tree.getParams().getHash(pop.transaction.publicationData.header);

  {
    // add cmd CheckPublicationData
    auto cmd = std::make_shared<CheckPublicationData>(
        pop.transaction.publicationData, tree, endorsedHash);
    cmds.push_back(std::move(cmd));
  }

  {
    // add cmd AddAltEndorsement
    auto e =
        AltEndorsement::fromContainerPtr(pop, containingHash, endorsedHash);
    auto cmd =
        std::make_shared<AddAltEndorsement>(tree.vbk(), tree, std::move(e));
    cmds.push_back(std::move(cmd));
  }
}

template <>
void payloadToCommands(VbkBlockTree& tree,
                       const VTB& pop,
                       const std::vector<uint8_t>& /* ignore */,
                       std::vector<CommandPtr>& cmds) {
  // process context blocks
  for (const auto& b : pop.transaction.blockOfProofContext) {
    addBlock(tree.btc(), b, pop.containingBlock.getHeight(), cmds);
  }
  // process block of proof
  addBlock(tree.btc(),
           pop.transaction.blockOfProof,
           pop.containingBlock.getHeight(),
           cmds);

  // add endorsement
  auto e = VbkEndorsement::fromContainerPtr(pop);
  auto cmd =
      std::make_shared<AddVbkEndorsement>(tree.btc(), tree, std::move(e));
  cmds.push_back(std::move(cmd));
}

template <>
std::vector<CommandGroup> payloadsToCommandGroups(
    VbkBlockTree& tree,
    const std::vector<VTB>& vtbs,
    const std::vector<uint8_t>& containinghash) {
  std::vector<CommandGroup> cgs;
  cgs.reserve(vtbs.size());

  vectorPopToCommandGroup<VbkBlockTree, VTB>(tree, vtbs, containinghash, cgs);

  return cgs;
}

}  // namespace altintegration
