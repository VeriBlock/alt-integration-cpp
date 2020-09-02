// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/blockchain/commands/commands.hpp>

namespace altintegration {

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

  std::vector<uint8_t> endorsed_hash =
      tree.getParams().getHash(pop.transaction.publicationData.header);

  auto e = AltEndorsement::fromContainerPtr(pop, containingHash, endorsed_hash);

  auto cmd =
      std::make_shared<AddAltEndorsement>(tree.vbk(), tree, std::move(e));
  cmds.push_back(std::move(cmd));
}

template <>
std::vector<CommandGroup> payloadsToCommandGroups(
    AltBlockTree& tree,
    const PopData& pop,
    const std::vector<uint8_t>& containinghash) {
  std::vector<CommandGroup> cgs;
  cgs.reserve(pop.context.size() + pop.vtbs.size() + pop.atvs.size());

  vectorPopToCommandGroup(tree, pop.context, containinghash, cgs);
  vectorPopToCommandGroup(tree, pop.vtbs, containinghash, cgs);
  vectorPopToCommandGroup(tree, pop.atvs, containinghash, cgs);

  return cgs;
}

template <>
void payloadToCommands(VbkBlockTree& tree,
                       const VTB& pop,
                       const std::vector<uint8_t>& /* ignore */,
                       std::vector<CommandPtr>& cmds) {
  // process context blocks
  for (const auto& b : pop.transaction.blockOfProofContext) {
    addBlock(tree.btc(), b, pop.containingBlock.height, cmds);
  }
  // process block of proof
  addBlock(tree.btc(),
           pop.transaction.blockOfProof,
           pop.containingBlock.height,
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