// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/storage/payloads_provider.hpp>

namespace altintegration {

bool PayloadsProvider::getCommands(AltTree& tree,
                                   const BlockIndex<AltBlock>& block,
                                   std::vector<CommandGroup>& out,
                                   ValidationState& state) {
  std::vector<ATV> atvs;
  atvs.reserve(block.getPayloadIds<ATV>().size());
  std::vector<VTB> vtbs;
  vtbs.reserve(block.getPayloadIds<VTB>().size());
  std::vector<VbkBlock> vbks;
  vbks.reserve(block.getPayloadIds<VbkBlock>().size());

  if (!getVBKs(block.getPayloadIds<VbkBlock>(), vbks, state)) {
    return false;
  }
  if (!getVTBs(block.getPayloadIds<VTB>(), vtbs, state)) {
    return false;
  }
  if (!getATVs(block.getPayloadIds<ATV>(), atvs, state)) {
    return false;
  }

  auto containingHash = block.getHash();
  vectorPopToCommandGroup<AltTree, VbkBlock>(tree, vbks, containingHash, out);
  vectorPopToCommandGroup<AltTree, VTB>(tree, vtbs, containingHash, out);
  vectorPopToCommandGroup<AltTree, ATV>(tree, atvs, containingHash, out);

  return true;
}

bool PayloadsProvider::getCommands(VbkBlockTree& tree,
                                   const BlockIndex<VbkBlock>& block,
                                   std::vector<CommandGroup>& out,
                                   ValidationState& state) {
  std::vector<VTB> vtbs;
  vtbs.reserve(block.getPayloadIds<VTB>().size());

  if (!getVTBs(block.getPayloadIds<VTB>(), vtbs, state)) {
    return false;
  }

  auto containingHash = block.getHash().asVector();
  vectorPopToCommandGroup<VbkBlockTree, VTB>(tree, vtbs, containingHash, out);

  return true;
}
}  // namespace altintegration
