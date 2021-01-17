// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/exceptions/state_corrupted.hpp>
#include <veriblock/storage/payloads_provider.hpp>

namespace altintegration {

void details::PayloadsReader::getCommands(AltBlockTree& tree,
                                          const BlockIndex<AltBlock>& block,
                                          std::vector<CommandGroup>& out,
                                          ValidationState& state) {
  PopData pop;
  if (!getContainingAltPayloads(block, pop, state)) {
    throw StateCorruptedException(block, state);
  }

  auto containingHash = block.getHash();
  // order is important!
  // first, add all VBK blocks, then add all VTBs, then add all ATVs
  vectorPopToCommandGroup<AltBlockTree, VbkBlock>(
      tree, pop.context, containingHash, out);
  vectorPopToCommandGroup<AltBlockTree, VTB>(
      tree, pop.vtbs, containingHash, out);
  vectorPopToCommandGroup<AltBlockTree, ATV>(
      tree, pop.atvs, containingHash, out);
}

void details::PayloadsReader::getCommands(VbkBlockTree& tree,
                                          const BlockIndex<VbkBlock>& block,
                                          std::vector<CommandGroup>& out,
                                          ValidationState& state) {
  std::vector<VTB> vtbs;
  if (!getContainingVbkPayloads(block, vtbs, state)) {
    throw StateCorruptedException(block, state);
  }

  auto containingHash = block.getHash().asVector();
  vectorPopToCommandGroup<VbkBlockTree, VTB>(tree, vtbs, containingHash, out);
}
}  // namespace altintegration
