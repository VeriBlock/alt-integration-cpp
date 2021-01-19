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
  const auto& atv_ids = block.getPayloadIds<ATV>();
  const auto& vtb_ids = block.getPayloadIds<VTB>();
  const auto& vbk_ids = block.getPayloadIds<VbkBlock>();

  std::vector<ATV> atvs;
  atvs.reserve(atv_ids.size());
  std::vector<VTB> vtbs;
  vtbs.reserve(vtb_ids.size());
  std::vector<VbkBlock> vbks;
  vbks.reserve(vbk_ids.size());

  for (size_t i = 0; i < atv_ids.size(); ++i) {
    ATV val;
    if (!getATV(atv_ids[i], val, state)) {
      throw StateCorruptedException(block, state);
    }
    atvs.push_back(val);
  }

  for (size_t i = 0; i < vtb_ids.size(); ++i) {
    VTB val;
    if (!getVTB(vtb_ids[i], val, state)) {
      throw StateCorruptedException(block, state);
    }
    vtbs.push_back(val);
  }

  for (size_t i = 0; i < vbk_ids.size(); ++i) {
    VbkBlock val;
    if (!getVBK(vbk_ids[i], val, state)) {
      throw StateCorruptedException(block, state);
    }
    vbks.push_back(val);
  }

  auto containingHash = block.getHash();
  // order is important! first, add all VBK blocks, then add all VTBs, then add
  // all ATVs
  vectorPopToCommandGroup<AltBlockTree, VbkBlock>(
      tree, vbks, containingHash, out);
  vectorPopToCommandGroup<AltBlockTree, VTB>(tree, vtbs, containingHash, out);
  vectorPopToCommandGroup<AltBlockTree, ATV>(tree, atvs, containingHash, out);
}

void details::PayloadsReader::getCommands(VbkBlockTree& tree,
                                          const BlockIndex<VbkBlock>& block,
                                          std::vector<CommandGroup>& out,
                                          ValidationState& state) {
  const auto& vtb_ids = block.getPayloadIds<VTB>();

  std::vector<VTB> vtbs;
  vtbs.reserve(vtb_ids.size());

  for (size_t i = 0; i < vtb_ids.size(); ++i) {
    VTB val;
    if (!getVTB(vtb_ids[i], val, state)) {
      throw StateCorruptedException(block, state);
    }
    vtbs.push_back(val);
  }

  auto containingHash = block.getHash().asVector();
  vectorPopToCommandGroup<VbkBlockTree, VTB>(tree, vtbs, containingHash, out);
}

void details::PayloadsWriter::writePayloads(const PopData& payloads) {
  writePayloads(payloads.atvs);
  writePayloads(payloads.vtbs);
  writePayloads(payloads.context);
}
}  // namespace altintegration
